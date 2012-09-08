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
	 * Create a PersistentObject that correponds to one that
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
	 * fully loaded.
	 */
	void load();

	/**
	 * Saves the state of the in-memory object to the
	 * database, overwriting the data in the database in the
	 * event of any conflict with the existing persisted data
	 * for this id. This is done by calling
	 * do_save_existing_partial (in the event the object is not
	 * fully loaded) or do_save_existing_all (in the event the object
	 * is fully loaded). The save_... functions should be defined in
	 * the derived class.
	 */
	void save_existing();

	/**
	 * Saves the state of the in-memory object to the database,
	 * as an additional item, rather than overwriting existing
	 * data. This is done by calling the pure virtual function
	 * do_save_new_all, which must be defined in the derived
	 * class.
	 */
	void save_new();

protected:

	boost::shared_ptr<DatabaseConnection> database_connection();

	Id id();

	void mark_as_persisted();

	void set_id(Id p_id);

private:

	virtual void do_load_all() = 0;

	virtual void do_save_existing_all() = 0;

	virtual void do_save_existing_partial() = 0;

	virtual void do_save_new_all() = 0;

	enum LoadingStatus
	{
		ghost,
		loading,
		loaded
	};

	// Data members


	boost::shared_ptr<DatabaseConnection> m_database_connection;
	boost::optional<Id> m_id;
	
	LoadingStatus m_loading_status;
	boost::optional<bool> m_has_been_persisted;

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
	m_has_been_persisted(false)
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
	if (m_loading_status == ghost)
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
void
PersistentObject<Id>::mark_as_persisted()
{
	m_has_been_persisted = true;
}


template <typename Id>
inline
void
PersistentObject<Id>::save_new()
{
	do_save_new_all();
	m_has_been_persisted = true;
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


}  // namespace sqloxx



#endif  // GUARD_persistent_object





