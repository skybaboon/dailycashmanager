
/** \file text_user_session.cpp
 *
 * \brief Contains source code relating to textual (console based)
 * user interface with Phatbooks application.
 *
 * \author Matthew Harvey
 * \date 04 July 2012.
 *
 * Copyright (c) 2012, Matthew Harvey. All rights reserved.
 */


#include "text_user_session.hpp"
#include "phatbooks_database_connection.hpp"
#include <boost/bind.hpp>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using boost::bind;
using std::cin;
using std::cout;
using std::endl;
using std::max;
using std::ostringstream;
using std::runtime_error;
using std::string;
using std::vector;

namespace phatbooks
{


void TextUserSession::say_hello()
{
	cout << "Hello!" << endl;
	return;
}

void TextUserSession::print_numbers()
{
	for (int i = 0; i != 5; ++i)
	{
		cout << i << '\t';
	}
	cout << endl;
	return;
}

void TextUserSession::quit()
{
	cout << "Exiting application..." << endl;
	return;
}

TextUserSession::TextUserSession():
	m_dummy_menu(new Menu),
	m_database_connection(new PhatbooksDatabaseConnection)
{
#warning unimplemented function body
	// Set up all the Menu objects.
	m_dummy_menu->add_item
	(	"Say hello",
		bind(&TextUserSession::say_hello, this)
	);
	m_dummy_menu->add_item
	(	"Print some numbers",
		bind(&TextUserSession::print_numbers, this)
	);
	m_dummy_menu->add_item
	(	"Quit",
		bind(&TextUserSession::quit, this),
		false,
		"x"
	);
}

TextUserSession::~TextUserSession()
{
}

void
TextUserSession::run()
{
	cout << endl << "Welcome to Phatbooks." << endl << endl;

#warning unimplemented function body

	// Get filename from user.
	// ...
	
	// Open connection with file.
	// ...
	
	// Setup file.
	// ...
	
	// Present the first menu.
	// ...
	m_dummy_menu->present_to_user();
	
	// Farewell the user.
	// ...
	
	return;
}


void
TextUserSession::Menu::add_item
(	string const& p_name,
	CallbackType p_callback,
	bool p_repeat_menu,
	string const& p_special_label
)
{
	MenuItem item(p_name, p_callback, p_repeat_menu, p_special_label);
	for
	(	ItemContainer::const_iterator it = m_items.begin();
		it != m_items.end();
		++it
	)
	{
		if (item.name() == it->name())
		{
			throw runtime_error("MenuItem with name already in Menu.");
		}
	}
	m_items.push_back(item);
	return;
}


void
TextUserSession::Menu::present_to_user()
{
	typedef vector<string>::size_type vec_sz;
	bool replay_menu = false;

	do
	{
		// Determine how to label each menu item
		int item_num = 1;
		string::size_type max_label_length = 0;
		vector<string> label_vec;
		ItemContainer::iterator it;
		for(it = m_items.begin(); it != m_items.end(); ++it)
		{
			if (it->has_special_label())
			{
				label_vec.push_back(it->special_label());
			}
			else
			{
				ostringstream oss;
				oss << item_num++;
				label_vec.push_back(oss.str());
			}
			max_label_length = max(label_vec.back().size(), max_label_length);
		}

		// Print the menu
		it = m_items.begin();
		cout << endl;
		for (vec_sz i = 0; i != label_vec.size(); ++i, ++it)
		{
			assert (it < m_items.end());
			cout << label_vec[i]
				 << string(max_label_length + 1 - label_vec[i].size(), ' ')
				 << it->name()
				 << endl;
		}

		// Receive and process user input
		for (bool successful = false; !successful; )
		{
			cout << endl << "Enter an option from the above menu: ";
			string input;
			while (!getline(cin, input))
			{
				cin.clear();
				cout << endl
				      << "There has been an error receiving your input. "
					  << "Please try again: ";
			}

			// See whether input corresponds to any of the item labels,
			// and invoke the item if it does.
			// This simple linear search is fast enough for all but
			// ridiculously large user menus.
			it = m_items.begin();
			for (vec_sz i = 0; i != label_vec.size(); ++i, ++it)
			{
				assert (it < m_items.end());
				if (input == label_vec[i])
				{
					it->invoke();
					replay_menu = it->repeat_menu();
					successful = true;
					break;
				}
			}
			if (!successful) cout << endl << "Please try again.";
		}
	}
	while (replay_menu);

	return;
}



TextUserSession::Menu::MenuItem::MenuItem
(	string const& p_name,
	Menu::CallbackType p_callback,
	bool p_repeat_menu,
	string const& p_special_label
):
	m_name(string()),
	m_callback(p_callback),
	m_repeat_menu(p_repeat_menu),
	m_special_label(p_special_label)
{
	if (p_name.empty())
	{
		throw runtime_error("p_name is empty.");
	}
	assert (p_name.size() > 0);
	m_name = p_name;
}


string
TextUserSession::Menu::MenuItem::name() const
{
	return m_name;
}


string
TextUserSession::Menu::MenuItem::special_label() const
{
	if (m_special_label.empty())
	{
		throw runtime_error("Item does not have special label.");
	}
	assert (!m_special_label.empty());
	return m_special_label;
}

bool
TextUserSession::Menu::MenuItem::repeat_menu() const
{
	return m_repeat_menu;
}

bool
TextUserSession::Menu::MenuItem::has_special_label() const
{
	return !m_special_label.empty();
}

void
TextUserSession::Menu::MenuItem::invoke()
{
	return m_callback();
}

bool
TextUserSession::Menu::MenuItem::operator<(MenuItem const& rhs) const
{
	return this->m_name < rhs.m_name;
}



}  // namespace phatbooks

