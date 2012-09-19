#ifndef GUARD_persistent_object_hpp
#define GUARD_persistent_object_hpp

#include "database_connection.hpp"
#include <jewel/decimal.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <string>


namespace sqloxx
{

template <typename Id>
class PersistentObject
{
public:

	/**
	 * Create a PersistentObject that corresponds to one that
	 * already exists in the database.
	 *
	 * @todo If the object doesn't exist, should this throw
	 * in the constructor?
	 */
	PersistentObject
	(	boost::shared_ptr<DatabaseConnection> p_database_connection,
		Id p_id
	);

	/** 
	 * Create a PersistentObject does not correspond to
	 * one that already exists in the database.
	 */
	explicit
	PersistentObject
	(	boost::shared_ptr<DatabaseConnection> p_database_connection
	);

	virtual ~PersistentObject();

	/**
	 * Calls the derived class's implementation
	 * of do_load_all, if the object is not already
	 * fully loaded. If the object does not have an id,
	 * then this function does nothing.
	 */
	void load();

	/**
	 * Saves the state of the in-memory object to the
	 * database, overwriting the data in the database in the
	 * event of any conflict with the existing persisted data
	 * for this id. This is done by calling
	 * do_save_existing_partial (in the event the object is not
	 * fully loaded) or do_save_existing_all (in the event the object
	 * is fully loaded). The do_save_... functions should be defined in
	 * the derived class.
	 */
	void save_existing();

	/**
	 * Saves the state of the in-memory object to the database,
	 * as an additional item, rather than overwriting existing
	 * data. This is done by calling the pure virtual function
	 * do_save_new_all, which must be defined in the derived
	 * class. Note the do_get_table_name function must also
	 * be defined in the derived class in order for this function
	 * to find an automatically generated id to assign to the object
	 * when saved. By default it is assumed that the id is an auto-
	 * incrementing integer primary key generated by SQLite. However this
	 * behaviour can be overridden by redefining the
	 * do_calculate_prospective_key function in the derived class.
	 */
	void save_new();

	/**
	 * Returns the id of the object. If the object doesn't have an id,
	 * this will CRASH via an assertion failure - rather than throw.
	 */
	Id id();

protected:

	boost::shared_ptr<DatabaseConnection> database_connection();

	/**
	 * Note an object that is created anew, that does not already exist
	 * in the database, should not have an id. By having an id, an object
	 * is saying "I exist in the database".
	 */
	void set_id(Id p_id);

	Id prospective_key();

	bool has_id();

private:

	virtual Id do_calculate_prospective_key();

	virtual void do_load_all() = 0;

	virtual void do_save_existing_all() = 0;

	virtual void do_save_existing_partial() = 0;

	virtual void do_save_new_all() = 0;

	virtual std::string do_get_table_name() = 0;

	enum LoadingStatus
	{
		ghost = 0,
		loading,
		loaded
	};


	// Data members


	boost::shared_ptr<DatabaseConnection> m_database_connection;
	boost::optional<Id> m_id;
	LoadingStatus m_loading_status;
};





template <typename Id>
inline
PersistentObject<Id>::PersistentObject
(	boost::shared_ptr<DatabaseConnection> p_database_connection,
	Id p_id
):
	m_database_connection(p_database_connection),
	m_id(p_id),
	m_loading_status(ghost)
{
}


template <typename Id>
inline
PersistentObject<Id>::PersistentObject
(	boost::shared_ptr<DatabaseConnection> p_database_connection
):
	m_database_connection(p_database_connection),
	m_loading_status(ghost)
{
}


template <typename Id>
inline
PersistentObject<Id>::~PersistentObject()
{
}


template <typename Id>
inline
void
PersistentObject<Id>::load()
{
	if (m_loading_status == ghost && has_id())
	{
		m_loading_status = loading;
		do_load_all();
		m_loading_status = loaded;
	}
	return;
}


template <typename Id>
inline
void
PersistentObject<Id>::save_existing()
{
	start:
	switch (m_loading_status)
	{
	case loaded:
		do_save_existing_all();
		break;
	case ghost:
		do_save_existing_partial();
		break;
	case loading:
		goto start;
		break;
	default:
		throw std::logic_error("Loading status not recognized.");
	}
	return;
}


template <typename Id>
inline
Id
PersistentObject<Id>::prospective_key()
{
	if (has_id())
	{
		throw std::logic_error
		(	"Object already has id so prospective_key does not apply."
		);
	}
	return do_calculate_prospective_key();
}


template <typename Id>
inline
Id
PersistentObject<Id>::do_calculate_prospective_key()
{	
	return database_connection()->template next_auto_key<Id>(do_get_table_name());
}


template <typename Id>
inline
void
PersistentObject<Id>::save_new()
{
	Id const key = prospective_key();
	do_save_new_all();
	set_id(key);
	return;
}


template <typename Id>
inline
boost::shared_ptr<DatabaseConnection>
PersistentObject<Id>::database_connection()
{
	return m_database_connection;
}


template <typename Id>
inline
Id
PersistentObject<Id>::id()
{
	return *m_id;
}

template <typename Id>
inline
void
PersistentObject<Id>::set_id(Id p_id)
{
	m_id = p_id;
	return;
}


template <typename Id>
inline
bool
PersistentObject<Id>::has_id()
{
	// Relies on the fact that m_id is a boost::optional<Id>, and
	// will convert to true if and only if it has been initialized.
	return m_id;
}


}  // namespace sqloxx



#endif  // GUARD_persistent_object





