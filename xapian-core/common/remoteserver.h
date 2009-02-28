/** @file remoteserver.h
 *  @brief Xapian remote backend server base class
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
 * Copyright (C) 2007,2009 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_REMOTESERVER_H
#define XAPIAN_INCLUDED_REMOTESERVER_H

#include "xapian/database.h"
#include "xapian/enquire.h"
#include "xapian/postingsource.h"
#include "xapian/visibility.h"

#include "remoteconnection.h"

#include <map>
#include <string>

using namespace std;

/** Remote backend server base class. */
class XAPIAN_VISIBILITY_DEFAULT RemoteServer : private RemoteConnection {
    /// Don't allow assignment.
    void operator=(const RemoteServer &);

    /// Don't allow copying.
    RemoteServer(const RemoteServer &);

    /** The database we're using.
     *
     *  If we're writable, this is the same as wdb.
     */
    Xapian::Database * db;

    /// The WritableDatabase we're using, or NULL if we're read-only.
    Xapian::WritableDatabase * wdb;

    /** Timeout for actions during a conversation.
     *
     *  The timeout is specified in milliseconds.  If the timeout is exceeded
     *  then a Xapian::NetworkTimeoutError is thrown.
     */
    Xapian::timeout active_timeout;

    /** Timeout while waiting for a new action from the client.
     *
     *  The timeout is specified in milliseconds.  If the timeout is exceeded
     *  then a Xapian::NetworkTimeoutError is thrown.
     */
    Xapian::timeout idle_timeout;

    /// Registered weighting schemes.
    map<string, Xapian::Weight *> wtschemes;

    /// Registered external posting sources.
    map<string, Xapian::PostingSource *> postingsources;

    /// Accept a message from the client.
    message_type get_message(Xapian::timeout timeout, string & result,
			     message_type required_type = MSG_MAX);

    /// Send a message to the client.
    void send_message(reply_type type, const string &message);

    // all terms
    void msg_allterms(const std::string & message);

    // get document
    void msg_document(const std::string & message);

    // term exists?
    void msg_termexists(const std::string & message);

    // get collection freq
    void msg_collfreq(const std::string & message);

    // get termfreq
    void msg_termfreq(const std::string & message);

    // get value statistics
    void msg_valuestats(const string & message);

    // keep alive
    void msg_keepalive(const std::string & message);

    // get doclength
    void msg_doclength(const std::string & message);

    // set the query; return the mset
    void msg_query(const std::string & message);

    // get termlist
    void msg_termlist(const std::string & message);

    // get postlist
    void msg_postlist(const std::string & message);

    // get positionlist
    void msg_positionlist(const std::string &message);

    // reopen
    void msg_reopen(const std::string & message);

    // get updated doccount and avlength
    void msg_update(const std::string &message);

    // commit
    void msg_commit(const std::string & message);

    // cancel
    void msg_cancel(const string &message);

    // add document
    void msg_adddocument(const std::string & message);

    // delete document
    void msg_deletedocument(const std::string & message);

    // delete document with unique term
    void msg_deletedocumentterm(const std::string & message);

    // replace document
    void msg_replacedocument(const std::string & message);

    // replace document with unique term
    void msg_replacedocumentterm(const std::string & message);

  public:
    /** Construct a RemoteServer.
     *
     *  @param dbpaths	The paths to the Xapian databases to use.
     *  @param fdin	The file descriptor to read from.
     *  @param fdout	The file descriptor to write to (fdin and fdout may be
     *			the same).
     *  @param active_timeout_	Timeout for actions during a conversation
     *			(specified in milliseconds).
     *  @param idle_timeout_	Timeout while waiting for a new action from
     *			the client (specified in milliseconds).
     *  @param writable Should the database be opened for writing?
     */
    RemoteServer(const std::vector<std::string> &dbpaths,
		 int fdin, int fdout,
		 Xapian::timeout active_timeout_,
		 Xapian::timeout idle_timeout_,
		 bool writable = false);

    /// Destructor.
    ~RemoteServer();

    /** Repeatedly accept messages from the client and process them.
     *
     *  The loop continues until either the connection is closed, or a
     *  non-Xapian exception is thrown.
     */
    void run();

    /// Register a user-defined weighting scheme class.
    void register_weighting_scheme(const Xapian::Weight &wt) {
	wtschemes[wt.name()] = wt.clone();
    }

    /** Register a user-defined posting source class.
     */
    void register_posting_source(const Xapian::PostingSource &source);
};

#endif // XAPIAN_INCLUDED_REMOTESERVER_H
