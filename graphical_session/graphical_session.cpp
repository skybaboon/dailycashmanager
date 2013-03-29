#include "graphical_session.hpp"
#include "application.hpp"
#include "b_string.hpp"
#include "wxs/app.hpp"
#include <boost/shared_ptr.hpp>
#include <string>

using boost::shared_ptr;
using std::string;
using std::wstring;

namespace phatbooks
{


namespace gui
{

GraphicalSession::GraphicalSession()
{
}

GraphicalSession::~GraphicalSession()
{
}

int
GraphicalSession::do_run()
{
	// This is a bit messy, but
	// we do this instead of using the IMPLEMENT_APP macro from
	// wxWidgets, because we don't want IMPLEMENT_APP to provide us
	// with a main function - we already have our own.
	wxApp* pApp = new App;
	wxApp::SetInstance(pApp);
	BString const app_name = Application::application_name();

	// The argv array required by wxEntryStart must be an array
	// of wchar_t*. We produce these as follows.
	wstring const argv0_w(app_name.begin(), app_name.end());

	// TODO Does this const_cast result in undefined behaviour?
	wchar_t* argv0_wct = const_cast<wchar_t*>(argv0_w.c_str());

	// We now construct the arguments required by wxEntryStart.
	wchar_t* argvs[] = { argv0_wct, 0 };
	int argca = 0;
	while (argvs[argca] != 0) ++argca;

	// At last...
	// TODO We need to invoke a wizard to assist the user
	// to create a file - or to open an existing file from within
	// the GUI. The invoking of the wizard should be done within
	// wxApp, and could be triggered by something in argvs.
	// Currently we get a crash in the attempt to populate the
	// AccountList from a non-existent database.
	wxEntryStart(argca, argvs);
	wxTheApp->OnInit();
	wxTheApp->OnRun();
	wxTheApp->OnExit();
	wxEntryCleanup();
	return 0;
}


int
GraphicalSession::do_run(string const& filepath_str)
{
	// TODO Validate the filepath here first - similar to what
	// we do in TextSession::do_run(...).
	
	// TODO There is code duplicated between here and the
	// other form of do_run

	// TODO Is this safe? We need to catch and handle if we can't open
	shared_ptr<PhatbooksDatabaseConnection> dbc
	(	new PhatbooksDatabaseConnection
	);
	dbc->open(filepath_str);

	App* pApp = new App;
	pApp->set_database_connection(dbc);
	wxApp::SetInstance(pApp);
	BString const app_name = Application::application_name();	

	// The argv array required by wxEntryStart must be an
	// array of wchar_t*. We produce these as follows.
	wstring const argv0_w(app_name.begin(), app_name.end());
	wstring const argv1_w(filepath_str.begin(), filepath_str.end());

	// TODO Do these const_cast's result in undefined behaviour?
	wchar_t* argv0_wct = const_cast<wchar_t*>(argv0_w.c_str());
	wchar_t* argv1_wct = const_cast<wchar_t*>(argv1_w.c_str());

	// We now construct the arguments required by wxEntryStart.
	wchar_t* argvs[] = { argv0_wct, argv1_wct, 0 };
	int argca = 0;
	while (argvs[argca] != 0) ++argca;

	// At last...
	wxEntryStart(argca, argvs);
	wxTheApp->OnInit();
	wxTheApp->OnRun();
	wxTheApp->OnExit();
	wxEntryCleanup();
	return 0;
}


}  // namespace gui
}  // namespace phatbooks
