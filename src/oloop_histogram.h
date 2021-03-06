#pragma once
#include "common.h"
#include "oloop.h"
#include "shuttle.h"
#include "querycommon.h"
#include "queryindexing.h"
#include "queryinterpreter.h"
#include "result.h"

namespace openset
{
	namespace db
	{
		class Table;
		class TablePartitioned;
	};

	namespace async
	{
		class OpenLoopHistogram : public OpenLoop
		{
		public:
			openset::query::Macro_s macros;
			ShuttleLambda<openset::result::CellQueryResult_s>* shuttle;
            std::string groupName;
			openset::db::Table* table;
            int64_t bucket; // scaled integater (double * 10000.0)
			openset::db::TablePartitioned* parts;
			int64_t maxLinearId;
			int32_t currentLinId;
			Person person;
			openset::query::Interpreter* interpreter;
			int instance;
			int runCount;
			int64_t startTime;
			int population;
			openset::query::Indexing indexing;
			openset::db::IndexBits* index;
			openset::result::ResultSet* result;
            // loop locals
            result::RowKey rowKey;

			explicit OpenLoopHistogram(
				ShuttleLambda<openset::result::CellQueryResult_s>* shuttle,
				openset::db::Table* table,
				openset::query::Macro_s macros, 
                std::string groupName,
                const int64_t bucket,
				openset::result::ResultSet* result,
				const int instance);

			~OpenLoopHistogram() final;

			void prepare() final;
			void run() final;
			void partitionRemoved() final;
		};
	}
}
