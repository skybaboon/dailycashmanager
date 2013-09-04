// Copyright (c) 2013, Matthew Harvey. All rights reserved.

#include "graphical_session.hpp"
#include "application.hpp"
#include "wxs/app.hpp"
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <jewel/log.hpp>
#include <wx/string.h>
#include <string>

using boost::shared_ptr;
using std::string;
using std::wstring;

namespace phatbooks
{


namespace gui
{

GraphicalSession::GraphicalSession():
	m_existing_application_instance_notified(false)
{
	JEWEL_LOG_TRACE();
}

GraphicalSession::~GraphicalSession()
{
	JEWEL_LOG_TRACE();
}

void
GraphicalSession::notify_existing_application_instance()
{
	JEWEL_LOG_TRACE();
	m_existing_application_instance_notified = true;
	return;
}

int
GraphicalSession::do_run()
{
	JEWEL_LOG_TRACE();

	shared_ptr<PhatbooksDatabaseConnection> dbc
	(	new PhatbooksDatabaseConnection
	);

	// This is a bit messy, but
	// we do this instead of using the IMPLEMENT_APP macro from
	// wxWidgets, because we don't want IMPLEMENT_APP to provide us
	// with a main function - we already have our own.
	App* pApp = new App;
	
	if (m_existing_application_instance_notified)
	{
		pApp->notify_existing_application_instance();
	}

	pApp->set_database_connection(dbc);
	wxApp::SetInstance(pApp);
	wxString const app_name = Application::application_name();

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
	wxEntryStart(argca, argvs);
	if (wxTheApp->OnInit())
	{
		wxTheApp->OnRun();
	}
	else
	{
		// User has cancelled rather than opening a file
		// Nothing to do.
	}
	wxTheApp->OnExit();
	wxEntryCleanup();

	JEWEL_LOG_TRACE();

	return 0;
}


int
GraphicalSession::do_run(string const& filepath_str)
{
	JEWEL_LOG_TRACE();

	// TODO Validate the filepath here first - similar to what
	// we did in TextSession::do_run(...) (see branches/tui_branch in
	// repository).
	
	// TODO There is code duplicated between here and the
	// other form of do_run

	// TODO Is this safe? We need to catch and handle if we can't open
	shared_ptr<PhatbooksDatabaseConnection> dbc
	(	new PhatbooksDatabaseConnection
	);
	boost::filesystem::path const filepath(filepath_str);

	App* pApp = new App;

	if (m_existing_application_instance_notified)
	{
		pApp->notify_existing_application_instance();
	}
	else
	{
		dbc->open(filepath);
	}

	pApp->set_database_connection(dbc);
	wxApp::SetInstance(pApp);
	wxString const app_name = Application::application_name();	

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

	JEWEL_LOG_TRACE();

	return 0;
}


}  // namespace gui
}  // namespace phatbooks
