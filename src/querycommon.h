#pragma once
#include "common.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "errors.h"
#include "dbtypes.h"
#include "attributes.h"
#include "var/var.h"
#include "../lib/str/strtools.h"

namespace openset
{
	namespace query
	{
		enum class BlockType_e
		{
			code,
			lambda,
			function
		};

		// Result Column Modifiers						
		enum class Modifiers_e : int32_t
		{
			sum,
			min,
			max,
			avg,
			count,
			dist_count_person,
			value,
			var,
			second_number,
			second_date,
			minute_number,
			minute_date,
			hour_number,
			hour_date,
			day_date,
			day_of_week,
			day_of_month,
			day_of_year,
			week_date,
			month_date,
			month_number,
			quarter_number,
			quarter_date,
			year_number,
			year_date,
		};

		enum class OpCode_e : int32_t
		{
			NOP = 0, // No operation

			PSHTBLCOL, // push column
			PSHRESCOL, // push result Column (may be grid, may be variable)
			//PSHRESGRP, // push group_by (may be grid, may be variable)
			VARIDX, // placeholder for an index to a variable
			PSHPAIR, // pushes a single pair dict to the stack
			PSHUSROBJ, // push object with deref
			PSHUSROREF, // push object with deref
			PSHUSRVAR, // push var
			PSHUSRVREF, // push pointer to variable (reference)
			PSHLITTRUE, // push True
			PSHLITFALSE, // push False
			PSHLITSTR, // push string
			PSHLITINT, // push integer
			PSHLITFLT, // push double
			PSHLITNUL, // push None value (NONE)

			POPUSROBJ, // pop object with deref
			POPUSRVAR, // pop variable
			POPTBLCOL, // pop column (place holder)
			POPRESGRP, // pop current group value
			POPRESCOL, // pop current result Column value

			CNDIF, // condition if
			CNDELIF, // condition else if
			CNDELSE, // condition else

			ITNEXT, // next iterator
			ITPREV, // prev iterator
			ITFOR, // for iterator

			MATHADD, // and last two stack items
			MATHSUB, // sub last two stack items
			MATHMUL, // multiply last two stack items
			MATHDIV, // divide last two stack items

			MATHADDEQ, // +=
			MATHSUBEQ, // -=
			MATHMULEQ, // *=
			MATHDIVEQ, // /=

			OPGT, // >
			OPLT, // <
			OPGTE, // >=
			OPLTE, // <-
			OPEQ, // == =
			OPNEQ, // != <>
			OPWTHN, // fuzzy range +/-
			OPNOT, // !

			LGCAND, // logical and
			LGCOR, // logical or

			MARSHAL, // Marshal an internal C++ function
			CALL, // Call a script defined function
			RETURN, // Pops the call stack leaves last item on stack

			TERM, // this script is done
			LGCNSTAND,
			LGCNSTOR
		};

		// Marshal Functions
		enum class Marshals_e : int64_t
		{
			marshal_tally,
			marshal_now,
			marshal_event_time,
			marshal_last_event,
			marshal_first_event,
			marshal_prev_match,
			marshal_first_match,
			marshal_bucket,
			marshal_round,
			marshal_trunc,
			marshal_fix,
			marshal_to_seconds,
			marshal_to_minutes,
			marshal_to_hours,
			marshal_to_days,
			marshal_get_second,
			marshal_round_second,
			marshal_get_minute,
			marshal_round_minute,
			marshal_get_hour,
			marshal_round_hour,
			marshal_round_day,
			marshal_get_day_of_week,
			marshal_get_day_of_month,
			marshal_get_day_of_year,
			marshal_round_week,
			marshal_round_month,
			marshal_get_month,
			marshal_get_quarter,
			marshal_round_quarter,
			marshal_get_year,
			marshal_round_year,
			marshal_iter_get,
			marshal_iter_set,
			marshal_iter_move_first,
			marshal_iter_move_last,
			marshal_iter_next,
			marshal_iter_prev,
			marshal_event_count,
			marshal_iter_within,
			marshal_iter_between,
			marshal_population,
			marshal_intersection,
			marshal_union,
			marshal_compliment,
			marshal_difference,
			marshal_session_count,
			marshal_return,
			marshal_break,
			marshal_continue,
			marshal_log,
			marshal_emit,
			marshal_schedule,
			marshal_debug,
			marshal_exit,
			marshal_init_dict,
			marshal_init_list,
			marshal_make_dict,
			marshal_make_list,
			marshal_set,
			marshal_list,
			marshal_dict,
			marshal_int,
			marshal_float,
			marshal_str,
			marshal_len,
			marshal_append,
			marshal_update,
			marshal_add,
			marshal_remove,
			marshal_del,
			marshal_contains,
			marshal_not_contains,
			marshal_pop,
			marshal_clear,
			marshal_keys,
			marshal_range, // not implemented
			marshal_str_split,
			marshal_str_find,
			marshal_str_rfind,
			marshal_str_replace,
			marshal_str_slice,
			marshal_str_strip,
			marshal_url_decode
		};

		// enum used for query index optimizer
		enum class HintOp_e : int64_t
		{
			UNSUPPORTED,
			PUSH_EQ,
			PUSH_NEQ,
			PUSH_GT,
			PUSH_GTE,
			PUSH_LT,
			PUSH_LTE,
			PUSH_PRESENT,
			PUSH_NOT,
			PUSH_NOP,
			BIT_OR,
			BIT_AND,
			NST_BIT_OR,
			NST_BIT_AND
		};
	}
}

namespace std
{
	/*
	 * RANT - being these enums are classes of POD types, they should have automatic 
	 *        hashing functions in GCC (they do in VC15+)
	 */

	template <>
	struct hash<openset::query::Modifiers_e>
	{
		size_t operator()(const openset::query::Modifiers_e& v) const
		{
			return static_cast<size_t>(v);
		}
	};

	template <>
	struct hash<openset::query::OpCode_e>
	{
		size_t operator()(const openset::query::OpCode_e& v) const
		{
			return static_cast<size_t>(v);
		}
	};

	template <>
	struct hash<openset::query::HintOp_e>
	{
		size_t operator()(const openset::query::HintOp_e& v) const
		{
			return static_cast<size_t>(v);
		}
	};

	template <>
	struct hash<openset::query::Marshals_e>
	{
		size_t operator()(const openset::query::Marshals_e& v) const
		{
			return static_cast<size_t>(v);
		}
	};
};

namespace openset
{
	namespace query
	{
		// String to Result Columns Modifier
		static const unordered_map<string, int64_t> TimeConstants = {
					{"seconds", 1'000},
					{"second", 1'000},
					{"minute", 60'000},
					{"minutes", 60'000},
					{"hour", 3'600'000},
					{"hours", 3'600'000},
					{"day", 86'400'000},
					{"days", 86'400'000}
			};

		static const unordered_map<string, int64_t> WithinConstants = {
					{"live", MakeHash("live")},
					{"first_event", MakeHash("first_event")},
					{"last_event", MakeHash("last_event")},
					{"prev_match", MakeHash("prev_match")},
					{"previous_match", MakeHash("previous_match")},
					{"first_match", MakeHash("first_match")},
			};

		enum class TimeSwitch_e : int
		{
			seconds,
			minutes,
			hours,
			days
		};

		static const unordered_map<string, TimeSwitch_e> TimeSwitchMap =
			{
					{"seconds", TimeSwitch_e::seconds},
					{"minutes", TimeSwitch_e::minutes},
					{"hours", TimeSwitch_e::hours},
					{"days", TimeSwitch_e::days}
			};

		// String to Result Columns Modifier
		static const unordered_map<string, Modifiers_e> ColumnModifiers = {
					{"sum", Modifiers_e::sum},
					{"min", Modifiers_e::min},
					{"max", Modifiers_e::max},
					{"avg", Modifiers_e::avg},
					{"count", Modifiers_e::count},
					{"dist_count_person", Modifiers_e::dist_count_person },
					{"value", Modifiers_e::value},
					{"val", Modifiers_e::value },
					{"variable", Modifiers_e::var },
					{"var", Modifiers_e::var},					
			};

		// Modifier to String (for debug output)
		static const unordered_map<Modifiers_e, string> ModifierDebugStrings = {
					{Modifiers_e::sum, "SUM"},
					{Modifiers_e::min, "MIN"},
					{Modifiers_e::max, "MAX"},
					{Modifiers_e::avg, "AVG"},
					{Modifiers_e::count, "COUNT"},
					{Modifiers_e::dist_count_person, "DCNTPP" },
					{Modifiers_e::value, "VALUE"},
					{Modifiers_e::var, "VAR"},
					{Modifiers_e::second_number, "SECOND"},
					{Modifiers_e::second_date, "DT_SECOND"},
					{Modifiers_e::minute_number, "MINUTE"},
					{Modifiers_e::minute_date, "DT_MINUTE"},
					{Modifiers_e::hour_number, "HOUR"},
					{Modifiers_e::hour_date, "DT_HOUR"},
					{Modifiers_e::day_date, "DT_DAY"},
					{Modifiers_e::day_of_week, "DAY_OF_WEEK"},
					{Modifiers_e::day_of_month, "DAY_OF_MONTH"},
					{Modifiers_e::day_of_year, "DAY_OF_YEAR"},
					{Modifiers_e::week_date, "DT_WEEK"},
					{Modifiers_e::month_date, "DT_MONTH"},
					{Modifiers_e::month_number, "MONTH"},
					{Modifiers_e::quarter_number, "QUARTER"},
					{Modifiers_e::quarter_date, "DT_QUARTER"},
					{Modifiers_e::year_number, "YEAR"},
					{Modifiers_e::year_date, "DT_YEAR"},
			};

		// opCode to String (for debug output)
		static const unordered_map<OpCode_e, string> OpDebugStrings =
			{
					{OpCode_e::NOP, "NOP"},

					{OpCode_e::PSHTBLCOL, "PSHTBLCOL"},
					{OpCode_e::PSHRESCOL, "PSHRESCOL"},
					{OpCode_e::VARIDX, "VARIDX"},
					{OpCode_e::PSHPAIR, "PSHPAIR"},
				//{opCode_e::PSHRESGRP, "PSHRESGRP"},
					{OpCode_e::PSHUSROBJ, "PSHUSROBJ"},
					{OpCode_e::PSHUSROREF, "PSHUSROREF" },
					{OpCode_e::PSHUSRVAR, "PSHUSRVAR"},
					{ OpCode_e::PSHUSRVREF, "PSHUSRVREF" },
					{OpCode_e::PSHLITSTR, "PSHLITSTR"},
					{OpCode_e::PSHLITINT, "PSHLITINT"},
					{OpCode_e::PSHLITFLT, "PSHLITFLT"},
					{OpCode_e::PSHLITNUL, "PSHLITNUL"},
					{ OpCode_e::PSHLITTRUE, "PSHLITTRUE" },
					{ OpCode_e::PSHLITFALSE, "PSHLITFALSE" },

					{OpCode_e::POPUSROBJ, "POPUSROBJ"},
					{OpCode_e::POPUSRVAR, "POPUSRVAR"},
					{OpCode_e::POPTBLCOL, "POPTBLCOL"},
					{OpCode_e::POPRESGRP, "POPRESGRP"},
					{OpCode_e::POPRESCOL, "POPRESCOL"},

					{OpCode_e::CNDIF, "CNDIF"},
					{OpCode_e::CNDELIF, "CNDELIF"},
					{OpCode_e::CNDELSE, "CNDELSE"},

					{OpCode_e::ITNEXT, "ITNEXT"},
					{OpCode_e::ITPREV, "ITPREV"},
					{OpCode_e::ITFOR, "ITFOR"},

					{OpCode_e::MATHADD, "MATHADD"},
					{OpCode_e::MATHSUB, "MATHSUB"},
					{OpCode_e::MATHMUL, "MATHMUL"},
					{OpCode_e::MATHDIV, "MATHDIV"},

					{ OpCode_e::MATHADDEQ, "OPADDEQ"},
					{ OpCode_e::MATHSUBEQ, "OPSUBEQ"},
					{ OpCode_e::MATHMULEQ, "OPMULEQ"},
					{ OpCode_e::MATHDIVEQ, "OPDIVEQ"},

					{OpCode_e::OPGT, "OPGT"},
					{OpCode_e::OPLT, "OPLT"},
					{OpCode_e::OPGTE, "OPGTE"},
					{OpCode_e::OPLTE, "OPLTE"},
					{OpCode_e::OPEQ, "OPEQ"},
					{OpCode_e::OPNEQ, "OPNEQ"},
					{OpCode_e::OPWTHN, "OPWTHN"},
					{OpCode_e::OPNOT, "OPNOT"},

					{OpCode_e::LGCAND, "LGCAND"},
					{OpCode_e::LGCOR, "LGCOR"},

					{OpCode_e::MARSHAL, "MARSHAL"},
					{OpCode_e::CALL, "CALL"},
					{OpCode_e::RETURN, "RETURN"},

					{OpCode_e::TERM, "TERM"}

			};

		static const unordered_map<string, Modifiers_e> TimeModifiers =
			{
					{"second", Modifiers_e::second_number},
					{"date_second", Modifiers_e::second_number},
					{"minute", Modifiers_e::minute_number},
					{"date_minute", Modifiers_e::minute_date},
					{"hour", Modifiers_e::hour_number},
					{"date_hour", Modifiers_e::hour_date},
					{"date_day", Modifiers_e::day_date},
					{"day_of_week", Modifiers_e::day_of_week},
					{"day_of_month", Modifiers_e::day_of_month},
					{"day_of_year", Modifiers_e::day_of_year},
					{"date_week", Modifiers_e::week_date},
					{"date_month", Modifiers_e::month_date},
					{"month", Modifiers_e::month_number},
					{"quarter", Modifiers_e::quarter_number},
					{"date_quarter", Modifiers_e::quarter_date},
					{"year", Modifiers_e::year_number},
					{"date_year", Modifiers_e::year_date},
			};

		static const unordered_set<Modifiers_e> isTimeModifiers =
			{
			};

		static const unordered_set<string> RedundantSugar =
			{
					{"of"},
					{"events"},
					{"event"}
			};

		using MarshalSet = std::unordered_set<openset::query::Marshals_e>;

		// Marshal maps
		static const unordered_map<string, Marshals_e> Marshals =
			{
					{"tally", Marshals_e::marshal_tally},
					{"now", Marshals_e::marshal_now},
					{"event_time", Marshals_e::marshal_event_time},
					{"last_event", Marshals_e::marshal_last_event},
					{"first_event", Marshals_e::marshal_first_event},
					{"prev_match", Marshals_e::marshal_prev_match},
					{"first_match", Marshals_e::marshal_first_match},
					{"bucket", Marshals_e::marshal_bucket},
					{"round", Marshals_e::marshal_round},
					{"trunc", Marshals_e::marshal_trunc},
					{"fix", Marshals_e::marshal_fix},
					{"to_seconds", Marshals_e::marshal_to_seconds},
					{"to_minutes", Marshals_e::marshal_to_minutes},
					{"to_hours", Marshals_e::marshal_to_hours},
					{"to_days", Marshals_e::marshal_to_days},
					{"get_second", Marshals_e::marshal_get_second},
					{"date_second", Marshals_e::marshal_round_second},
					{"get_minute", Marshals_e::marshal_get_minute},
					{"date_minute", Marshals_e::marshal_round_minute},
					{"get_hour", Marshals_e::marshal_get_hour},
					{"date_hour", Marshals_e::marshal_round_hour},
					{"date_day", Marshals_e::marshal_round_day},
					{"get_day_of_week", Marshals_e::marshal_get_day_of_week},
					{"get_day_of_month", Marshals_e::marshal_get_day_of_month},
					{"get_day_of_year", Marshals_e::marshal_get_day_of_year},
					{"date_week", Marshals_e::marshal_round_week},
					{"date_month", Marshals_e::marshal_round_month},
					{"get_month", Marshals_e::marshal_get_month},
					{"get_quarter", Marshals_e::marshal_get_quarter},
					{"date_quarter", Marshals_e::marshal_round_quarter},
					{"get_year", Marshals_e::marshal_get_year},
					{"date_year", Marshals_e::marshal_round_year},
					{"emit", Marshals_e::marshal_emit},
					{"schedule", Marshals_e::marshal_schedule},
					{"iter_get", Marshals_e::marshal_iter_get },
					{"iter_set", Marshals_e::marshal_iter_set },
					{"iter_move_first", Marshals_e::marshal_iter_move_first },
					{"iter_move_last", Marshals_e::marshal_iter_move_last },
					{"iter_next", Marshals_e::marshal_iter_next },
					{"iter_prev", Marshals_e::marshal_iter_prev },
					{"event_count", Marshals_e::marshal_event_count },
					{"iter_within", Marshals_e::marshal_iter_within},
					{"iter_between", Marshals_e::marshal_iter_between},
					{"population", Marshals_e::marshal_population },
					{"intersection", Marshals_e::marshal_intersection},
					{"union", Marshals_e::marshal_union},
					{"compliment", Marshals_e::marshal_compliment},
					{"difference", Marshals_e::marshal_difference},
					{"session_count", Marshals_e::marshal_session_count},
					{"return", Marshals_e::marshal_return},
					{"continue", Marshals_e::marshal_continue},
					{"break", Marshals_e::marshal_break},
					{"log", Marshals_e::marshal_log},
					{"debug", Marshals_e::marshal_debug},
					{"exit", Marshals_e::marshal_exit},
					{"__internal_init_dict", Marshals_e::marshal_init_dict},
					{"__internal_init_list", Marshals_e::marshal_init_list},
					{ "set", Marshals_e::marshal_set },
					{ "list", Marshals_e::marshal_list },
					{ "dict", Marshals_e::marshal_dict },
					{ "int", Marshals_e::marshal_int },
					{ "float", Marshals_e::marshal_float },
					{ "str", Marshals_e::marshal_str },
					{"__internal_make_dict", Marshals_e::marshal_make_dict},
					{"__internal_make_list", Marshals_e::marshal_make_list},
					{ "len", Marshals_e::marshal_len },
					{ "__append", Marshals_e::marshal_append },
					{ "__update", Marshals_e::marshal_update },
					{ "__add", Marshals_e::marshal_add },
					{ "__remove", Marshals_e::marshal_remove },
					{ "__del", Marshals_e::marshal_del },
					{ "__contains", Marshals_e::marshal_contains },
					{ "__notcontains", Marshals_e::marshal_not_contains },
					{ "__pop", Marshals_e::marshal_pop },
					{ "__clear", Marshals_e::marshal_clear },
					{ "__keys", Marshals_e::marshal_keys },
					{ "__split", Marshals_e::marshal_str_split },
					{ "__find", Marshals_e::marshal_str_find },
					{ "__rfind", Marshals_e::marshal_str_rfind },
					{ "__slice", Marshals_e::marshal_str_slice },
					{ "__strip", Marshals_e::marshal_str_strip},
					{ "range", Marshals_e::marshal_range },
					{ "url_decode", Marshals_e::marshal_url_decode }
			};

		static const unordered_set<Marshals_e> SegmentMathMarshals =
			{
					{Marshals_e::marshal_population },
					{Marshals_e::marshal_intersection},
					{Marshals_e::marshal_union},
					{Marshals_e::marshal_compliment},
					{Marshals_e::marshal_difference},
			};

		static const unordered_set<string> SessionMarshals =
			{
					"session_count",
			};

		// these are marshals that do not take params by default, so they appear
		// like variables.
		static const unordered_set<string> MacroMarshals =
			{
					{"now"},
					{"event_time"},
					{"last_event"},
					{"first_event"},
					{"prev_match"},
					{"first_match"},
					{"session_count"},
					{"__internal_init_dict"},
					{"__internal_init_list"},
			};

		// Comparatives
		static const unordered_map<string, OpCode_e> Operators = {
					{">=",OpCode_e::OPGTE},
					{"<=",OpCode_e::OPLTE},
					{">",OpCode_e::OPGT},
					{"<",OpCode_e::OPLT},
					{"==",OpCode_e::OPEQ},
					{"is",OpCode_e::OPEQ},
					{ "=",OpCode_e::OPEQ },
					{"!=",OpCode_e::OPNEQ},
					{"<>",OpCode_e::OPNEQ},
					{"not",OpCode_e::OPNOT},
					{"isnot",OpCode_e::OPNEQ},
					// {"within",opCode_e::OPWTHN},
			};

		static const unordered_map<string, OpCode_e> MathAssignmentOperators = {
			{ "+=", OpCode_e::MATHADDEQ }, // math operators
			{ "-=", OpCode_e::MATHSUBEQ },
			{ "*=", OpCode_e::MATHMULEQ },
			{ "/=", OpCode_e::MATHDIVEQ },
		};

		static const unordered_map<OpCode_e, string> OperatorsDebug = {
					{OpCode_e::OPGTE, ">="},
					{OpCode_e::OPLTE, "<="},
					{OpCode_e::OPGT, ">"},
					{OpCode_e::OPLT, "<"},
					{OpCode_e::OPEQ, "=="},
					{OpCode_e::OPNEQ, "!="},
					{OpCode_e::OPNOT, "!"}
					// {opCode_e::OPWTHN, "within"}
			};

		// Math
		static const unordered_map<string, OpCode_e> Math =
		{
				{"+", OpCode_e::MATHADD},
				{"-", OpCode_e::MATHSUB},
				{"*", OpCode_e::MATHMUL},
				{"/", OpCode_e::MATHDIV}
		};

		// Conditionals
		static const unordered_map<string, OpCode_e> LogicalOperators =
			{
					{"and", OpCode_e::LGCAND},
					{"or", OpCode_e::LGCOR},
					{"in", OpCode_e::LGCOR},
					{"nest_and", OpCode_e::LGCNSTAND},
					{"nest_or", OpCode_e::LGCNSTOR},
			};

		static const unordered_map<OpCode_e, string> LogicalOperatorsDebug = {
					{OpCode_e::LGCAND, "and"},
					{OpCode_e::LGCOR, "or"},
			};

		static const unordered_map<HintOp_e, string> HintOperatorsDebug = {
					{HintOp_e::UNSUPPORTED, "UNSUP"},
					{HintOp_e::PUSH_EQ, "PUSH_EQ"},
					{HintOp_e::PUSH_NEQ, "PUSH_NEQ"},
					{HintOp_e::PUSH_GT, "PUSH_GT"},
					{HintOp_e::PUSH_GTE, "PUSH_GTE"},
					{HintOp_e::PUSH_LT, "PUSH_LT"},
					{HintOp_e::PUSH_LTE, "PUSH_LTE"},
					{HintOp_e::PUSH_PRESENT, "PUSH_PRES"},
					{HintOp_e::PUSH_NOP, "PUSH_NOP"},
					{HintOp_e::BIT_OR, "BIT_OR"},
					{HintOp_e::BIT_AND, "BIT_AND"},
					{HintOp_e::NST_BIT_OR, "NST_BIT_OR"},
					{HintOp_e::NST_BIT_AND, "NST_BIT_AND"},

			};

		static const unordered_map<OpCode_e, HintOp_e> OpToHintOp = {
					{OpCode_e::OPGTE, HintOp_e::PUSH_GTE},
					{OpCode_e::OPLTE, HintOp_e::PUSH_LTE},
					{OpCode_e::OPGT, HintOp_e::PUSH_GT},
					{OpCode_e::OPLT, HintOp_e::PUSH_LT},
					{OpCode_e::OPEQ, HintOp_e::PUSH_EQ},
					{OpCode_e::OPNEQ, HintOp_e::PUSH_NEQ},
					{OpCode_e::OPNOT, HintOp_e::PUSH_NOT},
					{OpCode_e::LGCAND, HintOp_e::BIT_AND},
					{OpCode_e::LGCOR, HintOp_e::BIT_OR},
					{OpCode_e::LGCNSTOR, HintOp_e::NST_BIT_OR},
					{OpCode_e::LGCNSTAND, HintOp_e::NST_BIT_AND},
			};

		struct HintOp_s
		{
			HintOp_e op;
			string column;
			int64_t intValue;
			string textValue;
			bool numeric;

			HintOp_s(const HintOp_e op, 
				     const string column, 
				     const int64_t intValue) :
				op(op),
				column(column),
				intValue(intValue),
				numeric(true)
			{}

			HintOp_s(const HintOp_e op, 
				     const string column, 
				     const string text) :
				op(op),
				column(column),
				intValue(0),
				numeric(false)
			{
				if (text == "None") // special case
				{
					intValue = NONE;
					numeric = true;
				}
				else
				{
					textValue = text.substr(1, text.length() - 2);
					intValue = MakeHash(textValue);
				}
			}

			explicit HintOp_s(const HintOp_e op) :
				op(op),
				intValue(0),
				numeric(false)
			{}
		};

		using HintOpList = vector<HintOp_s>;

		struct Variable_s
		{
			string actual; // actual name
			string alias; // alias
			string space; // namespace
			string distinctColumnName{"__action"}; // name of column used for aggregators
			Modifiers_e modifier{Modifiers_e::value}; // default is value
			int index{-1}; // index
			int column{-1}; // column in grid
			int schemaColumn{-1}; // column in schema
			int distinctColumn{openset::db::COL_ACTION}; // column containing distinct key
			db::columnTypes_e schemaType{db::columnTypes_e::freeColumn};
			int popRefs{0}; // reference counter for pops
			int pushRefs{0}; // reference counter for pushes
			int sortOrder{-1}; // used for sorting in column order
			int lambdaIndex{-1}; // used for variable assignment by lambada
			bool nonDistinct{ false };

			cvar value{NONE};
			cvar startingValue{NONE};

			Variable_s()
			{}

			Variable_s(const string actual, 
					   const string space, 
				       const int sortOrder = -1):
				actual(actual),
				alias(actual),
				space(space),
				sortOrder(sortOrder)
			{}

			Variable_s(const string actual, 
					   const string alias,
			           const string space,
			           const Modifiers_e modifier = Modifiers_e::value,
			           const int sortOrder = -1):
				actual(actual),
				alias(alias),
				space(space),
				modifier(modifier),
				sortOrder(sortOrder)
			{}

			Variable_s(const Variable_s& source)
			{
				actual = source.actual;
				alias = source.alias;
				space = source.space;
				distinctColumnName = source.distinctColumnName;
				distinctColumn = source.distinctColumn;
				modifier = source.modifier;
				index = source.index;
				column = source.column;
				schemaColumn = source.schemaColumn;

				schemaType = source.schemaType;
				popRefs = source.popRefs;
				pushRefs = source.pushRefs;
				sortOrder = source.sortOrder;
				lambdaIndex = source.lambdaIndex;

				nonDistinct = source.nonDistinct;

				value = source.value;
				startingValue = source.startingValue;
			}
		};

		struct Debug_s
		{
			string text;
			string translation;
			int number;

			Debug_s() :
				number(-1)
			{}

			string toStr() const
			{
				return "@" + to_string(number) + " " + text;
			}

			string toStrShort() const
			{
				return "@" + to_string(number) + " " + trim(text, " \t");
			}
		};

		// structure fo final build
		struct Instruction_s
		{
			OpCode_e op;
			int64_t index;
			int64_t value;
			int64_t extra;
			Debug_s debug;

			Instruction_s(
				const OpCode_e op,
				const int64_t index,
				const int64_t value,
				const int64_t extra,
				Debug_s& dbg) :
				op(op),
				index(index),
				value(value),
				extra(extra),
				debug(dbg)
			{}

			Instruction_s(
				const OpCode_e op,
				const int64_t index,
				const int64_t value,
				const int64_t extra) :
				op(op),
				index(index),
				value(value),
				extra(extra),
				debug()
			{}
		};

		using InstructionList = vector<Instruction_s>;

		struct TextLiteral_s
		{
			int64_t hashValue; // xxhash of string
			int64_t index;
			string value;
		};

		using LiteralsList = vector<TextLiteral_s>;
		using VarList = vector<Variable_s>;
		using VarMap = unordered_map<string, Variable_s>;

		enum class sortOrder_e : int
		{
			ascending,
			descending
		};

		struct Function_s
		{
			string name;
			int64_t nameHash;
			int64_t execPtr;

			Function_s(const string functionName, const int64_t codePtr):
				name(functionName),
				nameHash(MakeHash(functionName)),
				execPtr(codePtr)
			{}
		};

		using FunctionList = vector<Function_s>;

		using ColumnLambdas = vector<int64_t>;

		struct Count_S
		{
			string name;
			int64_t functionHash;
		};

		using CountList = vector<Count_S>;

		// structure for variables
		struct Variables_S
		{
			VarList userVars;
			VarList tableVars;
			VarList columnVars;
			ColumnLambdas columnLambdas;
			FunctionList functions;
			LiteralsList literals;
			CountList countList;
		};

		using HintPair = pair<string, HintOpList>;
		using HintPairs = vector<HintPair>;
		using ParamVars = unordered_map<string, cvar>;
		using SegmentList = vector<std::string>;
		
		// struct containing compiled macro
		struct Macro_s
		{
			Variables_S vars;
			InstructionList code;
			HintPairs indexes;
			string segmentName;
			SegmentList segments;
			MarshalSet marshalsReferenced;

			int64_t segmentTTL{ -1 };
			int64_t segmentRefresh{ -1 };
			int sessionColumn{ -1 };
			int64_t sessionTime{ 60'000LL * 30LL }; // 30 minutes

			std::string rawScript;

			bool isSegment{ false };
			bool useGlobals{ false }; // uses global for table
			bool useCached{ false }; // for segments allow use of cached values within TTL
			bool isSegmentMath{ false }; // for segments, the index has the value, script execution not required
			bool useSessions{ false }; // uses session functions, we can cache these
		};

		using QueryPairs = vector<pair<string, Macro_s>>;
	}
}
