/*
 * This file is part of the Phatbooks project and is distributed under the
 * terms of the license contained in the file LICENSE.txt distributed
 * with this package.
 * 
 * Author: Matthew Harvey <matthew@matthewharvey.net>
 *
 * Copyright (c) 2012-2013, Matthew Harvey.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef GUARD_welcome_dialog_hpp_920675095599459
#define GUARD_welcome_dialog_hpp_920675095599459

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>

namespace phatbooks
{

// Begin forward declarations

class PhatbooksDatabaseConnection;

namespace gui
{

// End forward declarations

class WelcomeDialog: public wxDialog
{
public:

	WelcomeDialog(PhatbooksDatabaseConnection& p_database_connection);

	WelcomeDialog(WelcomeDialog const&) = delete;
	WelcomeDialog(WelcomeDialog&&) = delete;
	WelcomeDialog& operator=(WelcomeDialog const&) = delete;
	WelcomeDialog& operator=(WelcomeDialog&&) = delete;
	virtual ~WelcomeDialog();

	bool user_wants_new_file() const;

private:
	
	void configure_buttons();
	void on_existing_file_button_click(wxCommandEvent& event);
	void on_new_file_button_click(wxCommandEvent& event);
	void on_cancel_button_click(wxCommandEvent& event);

	PhatbooksDatabaseConnection& m_database_connection;
	bool m_user_wants_new_file;
	wxGridSizer* m_top_sizer;
	wxButton* m_existing_file_button;
	wxButton* m_new_file_button;

	// TODO LOW PRIORITY The cancel button looks ugly here. It would be better
	// to have a close box at the top right; however this was not showing when
	// I tried to add one using wxCLOSE_BOX style (at least not on
	// Fedora with Gnome). Hence the cancel button. Perhaps on Windows,
	// however, we can get the cancel button to work?
	wxButton* m_cancel_button;

	static int const s_existing_file_button_id = wxID_HIGHEST + 1;
	static int const s_new_file_button_id = wxID_HIGHEST + 2;

	DECLARE_EVENT_TABLE()

};  // WelcomeDialog


}  // namespace gui
}  // namespace phatbooks

#endif  // GUARD_welcome_dialog_hpp_920675095599459