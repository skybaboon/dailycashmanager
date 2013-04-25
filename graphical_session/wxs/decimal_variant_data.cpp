#include "decimal_variant_data.hpp"
#include "app.hpp"
#include "b_string.hpp"
#include "finformat.hpp"
#include <jewel/decimal.hpp>
#include <wx/intl.h>
#include <wx/string.h>
#include <cassert>
#include <istream>
#include <ostream>
#include <string>
#include <typeinfo>

using jewel::Decimal;
using std::istream;
using std::ostream;
using std::string;

namespace phatbooks
{
namespace gui
{

DecimalVariantData::DecimalVariantData(Decimal const& p_decimal):
	m_decimal(p_decimal)
{
}

bool
DecimalVariantData::Eq(wxVariantData& data) const
{
	assert (typeid(*this) == typeid(data));
	DecimalVariantData& dvdata =
		dynamic_cast<DecimalVariantData&>(data);
	return dvdata.m_decimal == m_decimal;
}

wxString
DecimalVariantData::GetType() const
{
	return wxString("jewel::Decimal");
}

bool
DecimalVariantData::Read(std::istream& stream)
{
	try
	{	
		string s;
		stream >> s;
		wxString wxs = std8_to_wx(s);
		App* app = dynamic_cast<App*>(wxTheApp);
		wxLocale const& locale = app->locale();
		m_decimal = wx_to_decimal(wxs, locale);
		return static_cast<bool>(stream);
	}
	catch (...)
	{
		return false;
	}
}

bool
DecimalVariantData::Read(wxString& string)
{
	try
	{
		App* app = dynamic_cast<App*>(wxTheApp);
		wxLocale const& locale = app->locale();
		m_decimal = wx_to_decimal(string, locale);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

bool
DecimalVariantData::Write(ostream& stream) const
{
	try
	{
		wxString wxs;
		App* app = dynamic_cast<App*>(wxTheApp);
		wxLocale const& locale = app->locale();
		wxs = finformat_wx(m_decimal, locale);
		stream << wx_to_std8(wxs);
		return static_cast<bool>(stream);
	}
	catch (...)
	{
		return false;
	}
}

bool
DecimalVariantData::Write(wxString& string) const
{
	assert (string.IsEmpty());
	try
	{
		App* app = dynamic_cast<App*>(wxTheApp);
		wxLocale const& locale = app->locale();
		string = finformat_wx(m_decimal, locale);
		return true;
	}
	catch (...)
	{
		return false;
	}
}


}  // namespace gui
}  // namespace phatbooks
