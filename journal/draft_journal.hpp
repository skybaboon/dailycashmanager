#ifndef GUARD_draft_journal_hpp
#define GUARD_draft_journal_hpp

#include "journal.hpp"
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include <vector>

namespace phatbooks
{

class PhatbooksDatabaseConnection;
class Repeater;

class DraftJournal: public Journal
{
public:

	/**
	 * Create the tables required for the persistence
	 * of DraftJournal instances in a SQLite database.
	 */
	static void setup_tables(PhatbooksDatabaseConnection& dbc);

	/**
	 * Initialize a "raw" DraftJournal, that will not yet
	 * correspond to any particular object in the database
	 */
	explicit
	DraftJournal
	(	boost::shared_ptr<PhatbooksDatabaseConnection> p_database_connection
	);

	/**
	 * Get a DraftJournal by id from the database.
	 */
	DraftJournal
	(	boost::shared_ptr<PhatbooksDatabaseConnection> p_database_connection,
		Id p_id
	);

	/**
	 * Create a DraftJournal from a Journal. Note the data members
	 * specific to DraftJournal will be uninitialized. All other
	 * members will be ***shallow-copied*** from p_journal.
	 */
	DraftJournal(Journal const& p_journal);

	/**
	 * Destructor.
	 */
	~DraftJournal();


	/**
	 * Set name of DraftJournal.
	 */
	void set_name(std::string const& p_name);
	
	/**
	 * Add a Repeater to the DraftJournal.
	 */
	void add_repeater(boost::shared_ptr<Repeater> repeater);
	
	/**
	 * @returns name of DraftJournal.
	 */
	std::string name();

	/**
	 * @todo Provide non-member swap and specialized std::swap per
	 * "Effective C++".
	 */
	void swap(DraftJournal& rhs);


private:

	/**
	 * Copy constructor - implemented, but deliberately private.
	 */
	DraftJournal(DraftJournal const& rhs);	

	void do_load();
	void do_save_existing();
	void do_save_new();

	/* Note this function is not redefined here as we want
	 * it to call Journal::do_get_table_name, which returns
	 * "journal", the name of the table that controls assignment
	 * of the id to all Journal instances, regardless of derived
	 * class.
	 */
	// std::string do_get_table_name() const;

	struct DraftJournalData
	{
		boost::optional<std::string> name;
		std::vector< boost::shared_ptr<Repeater> > repeaters;
	};

	boost::scoped_ptr<DraftJournalData> m_dj_data;
};

}  // namespace phatbooks


#endif  // GUARD_draft_journal_hpp
