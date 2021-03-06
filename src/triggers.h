#pragma once

#include "common.h"

#include "cjson/cjson.h"
#include <unordered_map>
#include "trigger.h"

namespace openset
{
	namespace db // name spaced forwards
	{
		class Table;
		class TablePartitioned;
		class Columns;
	};
};

namespace openset
{
	namespace trigger
	{

		class Triggers
		{
		private:
			openset::db::Table* table;
			openset::db::TablePartitioned* parts;
			openset::db::Columns* columns;
			std::unordered_map<int64_t, Trigger*> triggers;
			int64_t loadVersion;

		public:

			explicit Triggers(openset::db::TablePartitioned* parts);
			~Triggers();

			void load();

			std::unordered_map<int64_t, Trigger*>& getTriggerMap()
			{
				return triggers;
			}

			Trigger* getTrigger(int64_t triggerId)
			{
				auto iter = triggers.find(triggerId);
				return (iter == triggers.end()) ? nullptr : iter->second;
			}			

			void dispatchMessages() const;

			void checkForConfigChange();

		};
	};
};