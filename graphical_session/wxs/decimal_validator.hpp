#ifndef GUARD_decimal_validator_hpp
#define GUARD_decimal_validator_hpp

#include <jewel/decimal.hpp>
#include <wx/intl.h>
#include <wx/validate.h>



namespace phatbooks
{
namespace gui
{

class DecimalValidator: public wxValidator
{
public:
	DecimalValidator(jewel::Decimal const& p_decimal);
	DecimalValidator(DecimalValidator const& rhs);

	/**
	 * @param parent should point to an instance of wxTextCtrl.
	 */
	bool Validate(wxWindow* parent);

	bool TransferFromWindow();
	bool TransferToWindow();
	wxObject* Clone() const;

private:
	wxLocale const& locale() const;
	jewel::Decimal m_decimal;

};  // class DecimalValidator

}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_decimal_validator_hpp