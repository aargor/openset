#pragma once

#include <vector>
#include <regex>

#include "common.h"
#include "result.h"
#include "oloop.h"
#include "shuttle.h"
#include "var/var.h"
#include "dbtypes.h"

namespace openset
{
    namespace db
    {
        class Table;
        class TablePartitioned;
        class IndexBits;
    };

    namespace async
    {
        class OpenLoopColumn : public OpenLoop
        {
        public:
            enum class ColumnQueryMode_e : int
            {
                all,
                rx,
                sub,
                gt,
                gte,
                lt,
                lte,
                eq,
                between, // gte and lt
            };

            using Ids = vector<int64_t>;
            using GroupMap = unordered_map<int64_t,Ids>; // bucket -> IdList

            struct ColumnQueryConfig_s
            {
                std::string columnName;
                db::columnTypes_e columnType;
                int columnIndex;

                ColumnQueryMode_e mode { ColumnQueryMode_e::all };

                std::vector<std::string> segments{ "*" }; // efault is all

                // using cvars because I can put strings, bools, doubles and ints int them and
                // don't need to break out separate filter and bucket vars for 
                // each in this structure.
                cvar bucket{ 0 }; // histogramming
                cvar filterLow{ 0 };
                cvar filterHigh{ 0 };

                std::regex rx;
            };

            using SegmentNames = std::vector<std::string>;

        private:

            ShuttleLambda<result::CellQueryResult_s>* shuttle;

            ColumnQueryConfig_s config;

            db::Table* table;
            db::TablePartitioned* parts;
            result::ResultSet* result;

            int64_t stopBit{ 0 };
            int64_t instance{ 0 };

            std::vector<db::IndexBits*> segments;

            // loop locals
            result::RowKey rowKey;

            GroupMap groups;
            GroupMap::iterator groupsIter;

        public:

            explicit OpenLoopColumn(
                ShuttleLambda<result::CellQueryResult_s>* shuttle,
                openset::db::Table* table,
                ColumnQueryConfig_s config,
                openset::result::ResultSet* result,
                const int64_t instance);

            ~OpenLoopColumn() final;

            void prepare() final;
            void run() final;
            void partitionRemoved() final;
        };

    }
}