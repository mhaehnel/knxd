/*
    EIBD eib bus access and management daemon
    Copyright (C) 2005-2011 Martin Koegler <mkoegler@auto.tuwien.ac.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef GROUPCACHE_H
#define GROUPCACHE_H

#include <time.h>

#include "layer3.h"
#include "layer2.h"
#include "client.h"

class GroupCache;

struct GroupCacheEntry
{
  GroupCacheEntry(eibaddr_t dst) { this->dst = dst; }
  /** Layer 4 data */
  CArray data;
  /** source address */
  eibaddr_t src = 0;
  /** destination address */
  eibaddr_t dst;
  /** receive time */
  time_t recvtime;
};

typedef void (*GCReadCallback)(const GroupCacheEntry &foo, bool nowait, ClientConnPtr c);
typedef void (*GCLastCallback)(const Array<eibaddr_t> &foo, uint16_t end, ClientConnPtr c);

class GroupCacheReader
{
public:
  GroupCacheReader(GroupCache *); 
  ~GroupCacheReader();

  bool stopped = false;
  GroupCache *gc;
  virtual void updated(GroupCacheEntry *) = 0;
protected:
  virtual void stop();
};

class GroupCache:public Layer2mixin
{
  Array < GroupCacheReader * > reader;
  /** output queue */
  Array < GroupCacheEntry * > cache;
  /** controlled by .Start/Stop; if false, the whole code does nothing */
  bool enable = false;

public: // but only for GroupCacheReader
  bool init(Layer3 *l3);

  /** current position in 'updates' array */
  uint16_t pos;
  /** circular buffer of last-updated group addresses */
  eibaddr_t updates[0x100];
private:
  /** find this group */
  GroupCacheEntry *find (eibaddr_t dst);
  /** add this entry */
  void add (GroupCacheEntry * entry);
  /** signal that this entry has been updated */
  virtual void updated(GroupCacheEntry *);

public:
  /** constructor */
  GroupCache (TracePtr t);
  /** destructor */
  virtual ~GroupCache ();
  /** Feed data to the cache */
  void send_L_Data (L_Data_PDU * l);

  /** add a reader which gets triggered on updates */
  void add (GroupCacheReader *r);
  void remove (GroupCacheReader *r);

  /** Turn on caching, calls l3.registerGroupCallBack(ANY) */
  bool Start ();
  /** drop the whole cache */
  void Clear ();
  /** Turn off caching, deregisters */
  void Stop ();

  /** read, and optionally wait for, a cache entry for this address */
  void Read (eibaddr_t addr, unsigned timeout, uint16_t age,
             GCReadCallback cb, ClientConnPtr c);
  /** incrementally monitor group cache updates */
  void LastUpdates (uint16_t start, uint8_t timeout,
                    GCLastCallback cb, ClientConnPtr c);

  // remove this group
  void remove (eibaddr_t addr);
};

typedef std::shared_ptr<GroupCache> GroupCachePtr;

#endif
