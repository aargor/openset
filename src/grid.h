#pragma once

#include <vector>
#include <limits>
#include <unordered_map>
#include <cstring>

#include "common.h"
#include "cjson/cjson.h"

namespace openset
{
	namespace db
	{
		class Table;
		class Attributes;
		class AttributeBlob;

		const uint16_t TYPE_AND_MASK = 0b0111'0000'0000'0000;
		const uint16_t TYPE_CLEAR_MASK = 0b0000'1111'1111'1111;

		const int64_t int16_min = numeric_limits<int16_t>::min();
		const int64_t int16_max = numeric_limits<int16_t>::max();
		const int64_t int32_min = numeric_limits<int32_t>::min();
		const int64_t int32_max = numeric_limits<int32_t>::max();

		enum class columnType_e : uint16_t
		{
			row = 0, // 0 - new row - like \n
			null = 0b0001'0000'0000'0000, // 4096  - no data in cell
			copydown = 0b0010'0000'0000'0000, // 8192  - copy last rows cell
			int16 = 0b0011'0000'0000'0000, // 12288 - is int16 sized number
			int32 = 0b0100'0000'0000'0000, // 16384 - is int32 sized number
			int64 = 0b0101'0000'0000'0000 // 20480
		};

#pragma pack(push,1)
		/**
		This is the actual user record, Person is a class that
		can be assigned (Mounted) to one of these records, to perform
		operations.
		*/

		enum class flagType_e : int16_t
		{
			feature_eof = 0, // end of list
			feature_trigger = 1, // trigger
			future_trigger = 2, // scheduled trigger
		};

		struct Flags_s // 26 bytes.
		{
			int64_t reference; // what this flag refers to (i.e. a trigger ID)
			int64_t context; // value 1 (i.e. hash of function name)
			int64_t value; // value 2 (i.e. the future run-stamp of a trigger)
			flagType_e flagType;

			void set(
				const flagType_e flagType,
				const int64_t reference,
				const int64_t context,
				const int64_t value)
			{
				this->flagType = flagType;
				this->reference = reference;
				this->context = context;
				this->value = value;
			};

			// Note: flags are implemented using a Pool block and that pointer
			// is stored in a personData_s structure. The count is not part of
			// personData_s structure as instead the list is terminated with a
			// single 2 byte section filled with 0 (feature_eof). This allows us
			// to cast and check the buffer. 
		};

		struct PersonData_s
		{
			/*
			*  A User Record is a packed structure with the following layout
			*
			*  personData_s
			*  ------------
			*  idBytes
			*  ------------
			*  flags_s records
			*  ------------
			*  props
			*  ------------
			*  compressed event rows
			*
			*/

			int64_t id;
			int32_t linId;
			int32_t bytes; // bytes when uncompressed
			int32_t comp; // bytes when compressed
			int32_t propBytes;
			int16_t idBytes; // number of bytes in id string
			int16_t flagRecords; // number of flag records
			char events[1]; // char* (1st byte) of packed event struct

			std::string getIdStr() const
			{
				return std::string(events, idBytes);
			}

			// personData_s must already be sized, null not required
			void setIdStr(std::string &idString)
			{
				std::memcpy(events, idString.c_str(), idString.length());
				idBytes = idString.length();
			}

			void setIdStr(char* idString, const int len)
			{
				std::memcpy(events, idString, len);
				idBytes = len;
			}

			int64_t flagBytes() const
			{
				return (flagRecords * sizeof(Flags_s));
			}

			int64_t size() const
			{
				return sizeof(PersonData_s) + comp + propBytes + idBytes + flagBytes();
			}

			Flags_s* getFlags() 
			{
				if (!flagRecords)
					return nullptr;
				return reinterpret_cast<Flags_s*>(events + idBytes);
			}

			char* getIdPtr()
			{
				return events;
			}

			char* getProps() 
			{
				if (!propBytes)
					return nullptr;
				return events + idBytes + flagBytes();
			}

			char* getComp() 
			{
				return events + idBytes + flagBytes() + propBytes;
			}
		};

		struct Col_s
		{
			int64_t cols[MAXCOLUMNS];
		};
#pragma pack(pop)

		using Rows = vector<Col_s*>;

		class Grid
		{
		private:

			using LineNodes = vector<cjson*>;
			using ExpandedRows = vector<LineNodes>;

#pragma pack(push,1)
			struct Cast_s
			{
				int16_t columnNum; 
				int64_t val64;
			};
#pragma pack(pop)			

			const static int sizeOfCastHeader = sizeof(Cast_s::columnNum);
			const static int sizeOfCast = sizeof(Cast_s);

			// mapping
			int16_t columnMap[MAXCOLUMNS]; // TODO - FIX THIS
			int16_t reverseMap[MAXCOLUMNS]; // TODO - AND THIS
			int16_t isSet[MAXCOLUMNS]; // TODO - AND THIS

			unordered_map<int64_t, int32_t> insertMap;
			// we will get our memory via stack
			// so rows have tight cache affinity 
			HeapStack mem;
			Rows rows;
			PersonData_s* rawData{ nullptr };

			int32_t columnCount{ 0 };
			int32_t uuidColumn{ -1 }; // auto mapped in prepare
			int32_t sessionColumn{ -1 };
			int64_t sessionTime{ 60'000LL * 30LL }; // 30 minutes

			Table* table{ nullptr };
			Attributes* attributes{ nullptr };
			AttributeBlob* blob{ nullptr };

			int64_t rowBytes{ 0 };
			int64_t groupIdCounter{ Now() };

			bool fullSchemaMap{ false }; // only true on inserts and person queries

		public:
			Grid();
			~Grid();

			/**
			* \brief maps schema to the columnMap
			*
			* Why? The schema can have up to 8192 columns. These columns have
			* numeric indexes that allow allocated columns to be distributed
			* throughout that range. The Column map is a sequential list of
			* indexes into the actual schema, allowing us to create compact
			* grids that do not contain 8192 columns (which would be bulky
			* and slow)
			*/
			bool mapSchema(Table* tablePtr, Attributes* attributesPtr);
			bool mapSchema(Table* tablePtr, Attributes* attributesPtr, const vector<string>& columnNames);

			void setSessionTime(const int64_t sessionTime)
			{
				this->sessionTime = sessionTime;
			}

			void mount(PersonData_s* personData);
			void prepare();
			void insert(cjson* rowData);

			PersonData_s* addFlag(const flagType_e flagType, const int64_t reference, const int64_t context, const int64_t value);
			PersonData_s* clearFlag(const flagType_e flagType, const  int64_t reference, const int64_t context);

			/**
			* \brief re-encodes and compresses the row data after inserts
			*/
			PersonData_s* commit();

			// given an actual schema column, what is the 
			// column in the grid (which is compact)
			inline int getGridColumn(const int schemaColumn) const
			{
				return reverseMap[schemaColumn];
			}

			inline char* getUUIDcStr() const
			{
				return rawData ? rawData->events : nullptr;
			}

			inline string getUUIDString() const
			{
				return rawData ? string{ rawData->events } : "";
			}

			inline int64_t getUUID() const
			{
				return rawData->id;
			}

			inline int64_t getLinId() const
			{
				return rawData->linId;
			}

			inline bool isFullSchema() const
			{
				return fullSchemaMap;
			}

			inline const Rows* getRows() const
			{
				return &rows;
			}

			inline Attributes* getAttributes() const
			{
				return attributes;
			}

			AttributeBlob* getAttributeBlob() const;

			inline Table* getTable() const
			{
				return table;
			}

			cjson toJSON(const bool condensed = true) const;

			// brings object back to zero state
			void reinit();

		private:
			Col_s* newRow();
			/**
			* \brief reset rows (without resetting mappings)
			*
			* \note reset does not reset column mappings, we really
			*       want to do that once if we are re-using grids (or Person)
			*       objects.
			*/
			ExpandedRows iterate_expand(cjson* json) const;

			// ready object for re-use while maintaining mountings
			void reset();
		};
	};
};
