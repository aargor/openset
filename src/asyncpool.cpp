#include "common.h"
#include "asyncpool.h"
#include "config.h"
#include "file/file.h"
#include "internoderouter.h"
#include <cassert>

using namespace openset::async;

namespace openset
{
	namespace globals
	{
		openset::async::AsyncPool* async;
	}
};

int AsyncPool::getLeastBusy() const
{
	auto idx = 0;

	for (auto i = 0; i < workerMax; i++)
		if (workerInfo[i].jobs.size() <
			workerInfo[idx].jobs.size())
			idx = i;

	return idx;
}

void AsyncPool::mapPartitionsToAsyncWorkers()
{
	suspendAsync(); // pause all async workers for config changes

	// get the mapping for this node (OpenSetId)
	auto partitions = openset::globals::mapper->getPartitionMap()->getPartitionsByNodeId(globals::running->nodeId);

	for (auto p: partitions)
		initPartition(p);

	resumeAsync(); // resume all async workers

	if (!partitions.size())
		Logger::get().info("this node is empty, initialize it as a new cluster or join it to an existing cluster");
	else
		Logger::get().info("mapped " + to_string(partitions.size()) + " active partitions.");
}

void AsyncPool::suspendAsync()
{
	if (!running)
	{
		globalAsyncInitSuspend = true;
		globalAsyncLockDepth = 0;
		return;
	}

	csLock lock(globalAsyncLock);

	// get all async workers to suspend
	globalAsyncInitSuspend = true;

	// if we are not already suspended, loop until suspended is worker count
	if (globalAsyncSuspendedWorkerCount != workerMax)
		for (auto w = 0; w < workerMax; ++w)				
			workerInfo[w].conditional.notify_one();

	while (globalAsyncSuspendedWorkerCount != workerMax)
		this_thread::sleep_for(chrono::milliseconds(1));

	// increment the lock
	++globalAsyncLockDepth;
}

void AsyncPool::resumeAsync()
{
	if (!running)
	{
		globalAsyncInitSuspend = false;
		globalAsyncLockDepth = 0;
		return;
	}

	csLock lock(globalAsyncLock);

	--globalAsyncLockDepth;

	if (globalAsyncLockDepth == 0)
		globalAsyncInitSuspend = false;

	while (globalAsyncSuspendedWorkerCount != 0)
		this_thread::sleep_for(chrono::milliseconds(1));
}

void AsyncPool::waitForResume()
{
	while (true)
	{
		{
			csLock lock(globalAsyncLock);
			if (!globalAsyncLockDepth)
				return;
		}

		this_thread::sleep_for(chrono::milliseconds(1));
	}
}

void AsyncPool::assertAsyncLock() const
{
	Logger::get().fatal(globalAsyncInitSuspend, "LOCK NOT FOUND");
}


AsyncLoop* AsyncPool::initPartition(int32_t partition)
{
	/*
	 * This function factories a partition object, and assigns it to a worker thread
	 */
	assertAsyncLock();

	csLock lock(poolLock);

	// if this partition does not exist
	if (!partitions[partition])
	{
		// look for a worker with the least
		// assigned to it, we will add our new partition
		// here.
		auto listIdx = getLeastBusy();

		auto part = new partitionInfo_s(this, partition, listIdx);
		part->init();

		// add our new shard to the this worker thread
		workerInfo[listIdx].jobs.push_back(part);
		partitions[partition] = part;

		return part->ooLoop;
	}

	return partitions[partition]->ooLoop;
}

void AsyncPool::freePartition(int32_t partition)
{
	csLock lock(poolLock);

	if (partitions[partition])
	{
		// note we are orphaning the partition, it will
		// be cleaned up in the main job loop by asyncLoop
		// when it sees the markedForDeletion flag
		partitions[partition]->markedForDeletion = true;
		partitions[partition] = nullptr;
	}
}

void AsyncPool::cellFactory(std::vector<int> partitionList, function<OpenLoop*(AsyncLoop*)> factory)
{
	csLock lock(poolLock);

	for (auto pid : partitionList)
	{
		auto &p = partitions[pid];
		if (p)
		{
			auto cell = factory(p->ooLoop);

			// factory function can return nullptr if not applicable
			// i.e. query on a non-owner partition
			if (cell)
				p->ooLoop->queueCell(cell);
		}
		else
		{
			Logger::get().error("partition missing (" + to_string(pid) + ")");
		}
	}
}

void AsyncPool::cellFactory(function<OpenLoop*(AsyncLoop*)> factory)
{
	csLock lock(poolLock);

	for (auto& p : partitions)
		if (p)
		{
			auto cell = factory(p->ooLoop);

			// factory function can return nullptr if not applicable
			// i.e. query on a non-owner partition
			if (cell) 
				p->ooLoop->queueCell(cell);
		}
}

int32_t AsyncPool::count()
{
	csLock lock(poolLock);

	auto count = 0;
	for (auto p : partitions)
		if (p)
			++count;

	return count;
}

AsyncLoop* AsyncPool::isPartition(int32_t shardNumber)
{
	//csLock lock(poolLock); a lock probably isn't needed here

	if (partitions[shardNumber])
		return partitions[shardNumber]->ooLoop;

	return nullptr;
}

AsyncLoop* AsyncPool::getPartition(int32_t shardNumber)
{
	{
		csLock lock(poolLock);

		if (partitions[shardNumber])
			return partitions[shardNumber]->ooLoop;
	}

	return initPartition(shardNumber);
}

void AsyncPool::realtimeInc(int32_t shardNumber)
{
	if (partitions[shardNumber])
		++partitions[shardNumber]->realtimeCells;
}

void AsyncPool::realtimeDec(int32_t shardNumber)
{
	if (partitions[shardNumber])
		--partitions[shardNumber]->realtimeCells;
}

int32_t AsyncPool::getRealtimeRunning(int32_t shardNumber) const
{
	if (partitions[shardNumber])
		return partitions[shardNumber]->realtimeCells;
	else
		return 0;
}

void AsyncPool::runner(int32_t workerId) noexcept
{
	/*
	This is where the work happens.
	
	workerInfo contains information about
	which partitions a 'worker' is responsible for.

	Note: This worker is called by a thread, so it never
	exits and runs indefinitely,
	*/

	/* Playing with affinity masks... I don't think this helps
	auto logicalCPUs = std::thread::hardware_concurrency();

	auto threadHandle = GetCurrentThread();
	DWORD_PTR m_id = 0;
	DWORD_PTR m_mask = 1 << (workerId % logicalCPUs);

	SetThreadAffinityMask(threadHandle, m_mask);
	*/

	auto worker = &workerInfo[workerId];
	auto runAgain = 0;

	int64_t nextRun = -1;

	auto cleanup = [&]() -> bool
	{
		auto deletionCount = 0;

		// while we are suspended, lets check for partitions in need of deletion.
		for (auto iter = worker->jobs.begin(); iter != worker->jobs.end();)
		{
			if ((*iter)->markedForDeletion)
			{				
				auto t = *iter;
				iter = worker->jobs.erase(iter);
				delete t;
				++deletionCount;
			}
			else
				++iter;
		}

		return deletionCount;
	};

	while (true)
	{
		// are we forced to be idle (config change?)
		if (globalAsyncInitSuspend)
		{
			// indicate we are respecting the suspension 
			// using atomic for thread safe increments
			globalAsyncSuspendedWorkerCount += 1;

			// Loop & sleep until suspend is cleared 
			// while suspended check for deletions thread migrations
			while (globalAsyncInitSuspend)
			{			
				if (!cleanup()) // sleep only if we aren't deleting stuff
					this_thread::sleep_for(chrono::milliseconds(1));
			}

			globalAsyncSuspendedWorkerCount -= 1;
		}


		if (!runAgain)
		{ // we don't need the lock, so we will exit the moment we have it


			auto delay = (nextRun == -1) ? 250 : nextRun - Now();

			if (delay < 0) 
				delay = 0;

			if (delay && !worker->triggered)
			{
				// using a C++11 conditional lock (eventing) so that if
				// we aren't tasked up, we can wait for a cell to be added, or 250ms

				unique_lock<std::mutex> lock(worker->lock);
				worker->conditional.wait_for(lock, std::chrono::milliseconds(delay), [&]() -> bool
				{
					return (worker->triggered || globalAsyncInitSuspend);
				}); //std::chrono::milliseconds(delay) );
			}
			worker->triggered = false;
		}

		if (globalAsyncInitSuspend || globalAsyncLockDepth)
			continue;
		
		runAgain = 0;

		// jobs is a list of partitions
		// we will grab a job (s) and run it.

		nextRun = -1;

		for (auto s : worker->jobs)
		{
			if (s->markedForDeletion)
				continue;
			
			if (!openset::globals::mapper->getPartitionMap()->isMapped(
					s->ooLoop->getPartitionId(),
					openset::globals::running->nodeId))
				continue;

			// partitions contain open ended loops
			// we are going to run those loops here.
			if (s->ooLoop && s->ooLoop->Run(nextRun))
				++runAgain;			
		}

		if (runAgain)
			nextRun = 0;
	}
}

void AsyncPool::startAsync()
{
	/*
	These are the primary workers for our theaded-async model.
	each 'runner' will be responsible for a set of partitions
	*/

	if (!this->getPartitionMax()) // exit if there are no partitions
		return;

	

	Logger::get().info("Creating " + to_string(workerMax) + " partition pool threads.");

	vector<std::thread> workers;
	workers.reserve(workerMax);
	// make a little thread pool
	for (auto workerNumber = 0; workerNumber < workerMax; workerNumber++)
	{
		workers.push_back(thread(
			&AsyncPool::runner,
			this,
			workerNumber));
	}

	running = true;
	ThreadSleep(1000);

	// wait for them to be finished
	for (auto w = 0; w < workers.size(); w++)
		workers[w].detach();


}
