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

/**
 * Class template for creating objects persisted to a database.
 *
 * @todo Provide for atomicity of loading and saving (not just of
 * SQL execution, but of the actual alteration of the in-memory objects).
 *
 * @todo Unit testing.
 *
 * @todo Document protected function APIs.
 */
template <typename Id>
class PersistentObject
{
public:

	/**
	 * Create a PersistentObject that corresponds (or purports to correspond)
	 * to one that already exists in the database.
	 *
	 * Note that even if there is no corresponding object in the database for
	 * the given value p_id, this constructor will still proceed without
	 * complaint. The constructor does not actually perform any checks on the
	 * validity either of p_database_connection or of p_id.
	 *
	 * Exception safety: <em>nothrow guarantee</em> (though derived classes'
	 * constructors might, of course, throw).
	 */
	PersistentObject
	(	boost::shared_ptr<DatabaseConnection> p_database_connection,
		Id p_id
	);

	/** 
	 * Create a PersistentObject that does not correspond to
	 * one that already exists in the database.
	 *
	 * Exception safety: <em>nothrow guarantee</em> (though derived classes'
	 * constructors might, of course, throw).
	 */
	explicit
	PersistentObject
	(	boost::shared_ptr<DatabaseConnection> p_database_connection
	);

	/**
	 * Exception safety: <em>nothrow guarantee</em> (though this destructor
	 * cannot, of course, offer any guarantees about the exception safety
	 * of derived classes' destructors).
	 */
	virtual ~PersistentObject();

	/**
	 * Calls the derived class's implementation
	 * of do_load_all, if the object is not already
	 * fully loaded. If the object does not have an id,
	 * then this function does nothing.
	 *
	 * Note the implementation is wrapped as a transaction
	 * by calls to begin_transaction and end_transaction
	 * methods of the DatabaseConnection.
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
	 *
	 * Note the implementation is wrapped as a transaction
	 * by calls to begin_transaction and end_transaction
	 * methods of the DatabaseConnection.
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
	 *
	 * Note the implementation is wrapped as a transaction by call to
	 * begin_transaction and end_transaction methods of
	 * DatabaseConnection.
	 */
	void save_new();

	/**
	 * Returns the id of the object. If the object doesn't have an id,
	 * this will CRASH via an assertion failure - rather than throw.
	 */
	Id id() const;

protected:

	/**
	 * @returns a boost::shared_ptr to the database connection with which
	 * this instance of PersistentObject is associated. This is where the
	 * object will be loaded from or saved to, as the case may be.
	 *
	 * Exception safety: <em>nothrow guarantee</em>
	 */
	boost::shared_ptr<DatabaseConnection> database_connection() const;

	/**
	 * Note an object that is created anew, that does not already exist
	 * in the database, should not have an id. By having an id, an object
	 * is saying "I exist in the database".
	 */
	void set_id(Id p_id);

	/**
	 * @returns the id that would be assigned to the this instance of
	 * PersistentObject when saved to the database.
	 *
	 * This function calls /e do_calculate_prospective_key, which has a
	 * default implementation but may be redefined.
	 *
	 * @throws std::logic_error in the event this instance already has
	 * an id. (This occurs regardless of how/whether
	 * \e do_calculate_prospective_key is redefined.)
	 *
	 * Apart from \e std::logic_error as just described, the exception
	 * throwing behaviour and exception safety of this function depend on
	 * those of the function PersistentObject::do_calculate_prospective_key.
	 */
	Id prospective_key() const;

	/**
	 * @returns \e true if this instance of PersistentObject has
	 * an valid id; otherwise returns \e false.
	 *
	 * Exception safety: <em>nothrow guarantee</em>.
	 */
	bool has_id() const;

	/**
	 * <em>The documentation for this function refers only to the
	 * default definition provided by PersistentObject.</em>
	 *
	 * Provides an implementation for the public function prospective_key.
	 * Should not be called by any functions are than prospective_key.
	 *
	 * @throws sqloxx::TableSizeException if the greatest primary key value
	 * already in the table (i.e. the table into which this instance of
	 * PersistentObject would be persisted) is the maximum value for the
	 * type \e Id, so that another row could not be inserted without overflow.
	 *
	 * @throws sqloxx::DatabaseException, or a derivative therefrom, may
	 * be thrown if there is some other
	 * error finding the next primary key value. This should not occur except
	 * in the case of a corrupt database, or a memory allocation error
	 * (extremely unlikely), or the database connection being invalid
	 * (including because not yet connected to a database file).
	 * @throws sqloxx::InvalidConnection if the database connection
	 * associated with this instance of PersistentObject is invalid.
	 *
	 * Exception safety: <em>strong guarantee</em>.
	 */
	virtual Id do_calculate_prospective_key() const;

	/**
	 * See documentation for public \e load function.
	 *
	 * Exception safety: <em>depends on function definition
	 * provided by derived class</em>
	 */
	virtual void do_load_all() = 0;

	/**
	 * See documentation for public <em>save_existing</em> function.
	 *
	 * Exception safety: <em>depends on function definition provided by
	 * derived class</em>.
	 */
	virtual void do_save_existing_all() = 0;

	/**
	 * See documentation for public <em>save_existing</em> function.
	 *
	 * Exception safety: <em>depends on function definition provided by
	 * derived class</em>.
	 */
	virtual void do_save_existing_partial() = 0;

	/**
	 * See documentation for public <em>save_new</em> function.
	 *
	 * Exception safety: <em>depends on function definition provided by
	 * derived class</em>.
	 */
	virtual void do_save_new_all() = 0;

	/**
	 * This function should be defined in the derived class to return the
	 * name of the table in which instances of the derived class are stored
	 * in the database. This function is in turn called by the default
	 * implementation of \e do_calculate_prospective_key, which is in turn
	 * called by \e save_new.
	 *
	 * Exception safety: <em>depends on function definition provided by
	 * derived class</em>.
	 */
	virtual std::string do_get_table_name() const = 0;

private:

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
		m_database_connection->begin_transaction();
		do_load_all();
		m_database_connection->end_transaction();
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
		m_database_connection->begin_transaction();
		do_save_existing_all();
		m_database_connection->end_transaction();
		break;
	case ghost:
		m_database_connection->begin_transaction();
		do_save_existing_partial();
		m_database_connection->end_transaction();
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
PersistentObject<Id>::prospective_key() const
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
PersistentObject<Id>::do_calculate_prospective_key() const
{	
	return database_connection()->template next_auto_key<Id>
	(	do_get_table_name()
	);
}


template <typename Id>
inline
void
PersistentObject<Id>::save_new()
{
	m_database_connection->begin_transaction();
	Id const key = prospective_key();
	do_save_new_all();
	m_database_connection->end_transaction();
	set_id(key);
	return;
}


template <typename Id>
inline
boost::shared_ptr<DatabaseConnection>
PersistentObject<Id>::database_connection() const
{
	return m_database_connection;
}


template <typename Id>
inline
Id
PersistentObject<Id>::id() const
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
PersistentObject<Id>::has_id() const
{
	// Relies on the fact that m_id is a boost::optional<Id>, and
	// will convert to true if and only if it has been initialized.
	return m_id;
}


}  // namespace sqloxx



#endif  // GUARD_persistent_object





