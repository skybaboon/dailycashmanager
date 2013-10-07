// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#ifndef GUARD_repeater_table_iterator_hpp_8880911641233094
#define GUARD_repeater_table_iterator_hpp_8880911641233094

#include "repeater.hpp"
#include <sqloxx/handle.hpp>
#include <sqloxx/table_iterator.hpp>

namespace phatbooks
{

typedef
	sqloxx::TableIterator<sqloxx::Handle<Repeater> >
	RepeaterTableIterator;

}  // namespace phatbooks

#endif  // GUARD_repeater_table_iterator_hpp_8880911641233094
