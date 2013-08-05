#include "gridded_scrolled_panel.hpp"
#include "finformat.hpp"
#include "locale.hpp"
#include "phatbooks_database_connection.hpp"
#include "sizing.hpp"
#include <jewel/decimal.hpp>
#include <wx/gbsizer.h>
#include <wx/gdicmn.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/window.h>
#include <cassert>


namespace phatbooks
{
namespace gui
{

GriddedScrolledPanel::GriddedScrolledPanel
(	wxWindow* p_parent,
	wxSize const& p_size,
	PhatbooksDatabaseConnection& p_database_connection,
	bool p_horizontal_scrolling_enabled
):
	wxScrolledWindow
	(	p_parent,
		wxID_ANY,
		wxDefaultPosition,
		p_size,
		(p_horizontal_scrolling_enabled? (wxVSCROLL | wxHSCROLL): wxVSCROLL)
	),
	m_current_row(0),
	m_top_sizer(0),
	m_database_connection(p_database_connection)
{
	m_top_sizer = new wxGridBagSizer(standard_gap(), standard_gap());
	SetSizer(m_top_sizer);
	int const standard_scrolling_increment = 10;
	int const horizontal_scrolling_rate =
		(p_horizontal_scrolling_enabled? standard_scrolling_increment: 0);
	SetScrollRate(horizontal_scrolling_rate, standard_scrolling_increment);
}

GriddedScrolledPanel::~GriddedScrolledPanel()
{
}

wxGridBagSizer&
GriddedScrolledPanel::top_sizer()
{
	assert (m_top_sizer);
	return *m_top_sizer;
}

int
GriddedScrolledPanel::current_row() const
{
	return m_current_row;
}

void
GriddedScrolledPanel::increment_row()
{
	++m_current_row;
	return;
}

void
GriddedScrolledPanel::decrement_row()
{
	--m_current_row;
	return;
}

void
GriddedScrolledPanel::display_text
(	wxString const& p_text,
	int p_column,
	int p_alignment_flags
)
{
	wxStaticText* text = new wxStaticText
	(	this,
		wxID_ANY,
		p_text,
		wxDefaultPosition,
		wxDefaultSize,
		p_alignment_flags
	);
	top_sizer().Add
	(	text,
		wxGBPosition(current_row(), p_column),
		wxDefaultSpan,
		p_alignment_flags
	);
	return;
}

void
GriddedScrolledPanel::display_decimal
(	jewel::Decimal const& p_decimal,
	int p_column
)
{
	wxStaticText* text = new wxStaticText
	(	this,
		wxID_ANY,
		finformat_wx(p_decimal, locale()),
		wxDefaultPosition,
		wxDefaultSize,
		wxALIGN_RIGHT
	);
	top_sizer().Add
	(	text,
		wxGBPosition(current_row(), p_column),
		wxDefaultSpan,
		wxALIGN_RIGHT
	);
	return;
}

PhatbooksDatabaseConnection&
GriddedScrolledPanel::database_connection()
{
	return m_database_connection;
}

PhatbooksDatabaseConnection const&
GriddedScrolledPanel::database_connection() const
{
	return m_database_connection;
}


}  // namespace gui
}  // namespace phatbooks
