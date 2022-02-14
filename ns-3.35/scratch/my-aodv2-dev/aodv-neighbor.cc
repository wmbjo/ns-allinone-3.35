/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Based on
 *      NS-2 AODV model developed by the CMU/MONARCH group and optimized and
 *      tuned by Samir Das and Mahesh Marina, University of Cincinnati;
 *
 *      AODV-UU implementation by Erik Nordström of Uppsala University
 *      http://core.it.uu.se/core/index.php/AODV-UU
 *
 * Authors: Elena Buchatskaia <borovkovaes@iitp.ru>
 *          Pavel Boyko <boyko@iitp.ru>
 */
#include <iostream> 
#include <algorithm>
#include "ns3/log.h"
#include "ns3/wifi-mac-header.h"
#include "aodv-neighbor.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AodvNeighbors2");

namespace aodv {
Neighbors::Neighbors (Time delay)
  : m_ntimer (Timer::CANCEL_ON_DESTROY)
{
  m_ntimer.SetDelay (delay);
  m_ntimer.SetFunction (&Neighbors::Purge, this);
  m_txErrorCallback = MakeCallback (&Neighbors::ProcessTxError, this);
}

bool
Neighbors::IsNeighbor (Ipv4Address addr)
{
  std::cout << "IsNeighbor" << std::endl;
  Purge ();
  for (std::vector<Neighbor>::const_iterator i = m_nb.begin ();
       i != m_nb.end (); ++i)
    {
      if (i->m_neighborAddress == addr)
        {
          return true;
        }
    }
  return false;
  //NS_LOG_LOGIC ("IS NEIGHBOR" << addr);
}

Time
Neighbors::GetExpireTime (Ipv4Address addr)
{
  std::cout << "GetExpireTime" << std::endl;
  Purge ();
  for (std::vector<Neighbor>::const_iterator i = m_nb.begin (); i
       != m_nb.end (); ++i)
    {
      if (i->m_neighborAddress == addr)
        {
          return (i->m_expireTime - Simulator::Now ());
        }
    }
  return Seconds (0);
}

void Neighbors::Printn()
{
  std::cout << "Printn" << std::endl;
  for (std::vector<Neighbor>::iterator i = m_nb.begin (); i != m_nb.end (); ++i)
  { 
    Ipv4Address addr = i->m_neighborAddress;
    std::ofstream nextMessageOut; 
    nextMessageOut.open("tempMessage.txt");
    //nextMessage = "xxxxxxxxx";
    addr.Print(nextMessageOut);
    nextMessageOut.close();
    std::ifstream nextMessageIn;
    nextMessageIn.open("tempMessage.txt");
    std::string addrs;
    getline(nextMessageIn, addrs);
    //std::cout << nextMessage <<'\n';
    NS_LOG_LOGIC ("Address ----> " << addrs);
  }
}


// void Neighbors::Printv(Ptr<OutputStreamWrapper> stream)const
//  {
//     //for (int i = 0; i < stream.size(); i++) {
//         //std::cout << input.at(i) << ' ';
//         //std::cout << typeid(stream) << '\n';
//     //}
//    } 

void
Neighbors::Update (Ipv4Address addr, Time expire)
{
  std::cout << "Update" << std::endl;
  for (std::vector<Neighbor>::iterator i = m_nb.begin (); i != m_nb.end (); ++i)
    {
      
      if (i->m_neighborAddress == addr)
        {
          // std::cout << "*******" << addr;
          i->m_expireTime
            = std::max (expire + Simulator::Now (), i->m_expireTime);
            
          if (i->m_hardwareAddress == Mac48Address ())
            {
              i->m_hardwareAddress = LookupMacAddress (i->m_neighborAddress);
            }
          return;
        }
    }
  //NS_LOG_LOGIC ("************** " << std::vector<Neighbor>::iterator i = m_nb.begin ());
  NS_LOG_LOGIC ("Open link to " << addr);
  NS_LOG_LOGIC ("expire time " << expire << typeid(expire).name());
  Neighbor neighbor (addr, LookupMacAddress (addr), expire + Simulator::Now ());
  m_nb.push_back (neighbor);
  Purge ();
}

/**
 * \brief CloseNeighbor structure
 */
struct CloseNeighbor
{
  /**
   * Check if the entry is expired
   *
   * \param nb Neighbors::Neighbor entry
   * \return true if expired, false otherwise
   */
  bool operator() (const Neighbors::Neighbor & nb) const
  {
    return ((nb.m_expireTime < Simulator::Now ()) || nb.close);
  }
};

void
Neighbors::Purge ()
{
  std::cout << "Purge" << std::endl;
  if (m_nb.empty ())
    {
      return;
    }

  CloseNeighbor pred;
  if (!m_handleLinkFailure.IsNull ())
    {
      for (std::vector<Neighbor>::iterator j = m_nb.begin (); j != m_nb.end (); ++j)
        {
          if (pred (*j))
            {
              NS_LOG_LOGIC ("Close link to " << j->m_neighborAddress);
              m_handleLinkFailure (j->m_neighborAddress);
            }
        }
    }
  m_nb.erase (std::remove_if (m_nb.begin (), m_nb.end (), pred), m_nb.end ());
  m_ntimer.Cancel ();
  m_ntimer.Schedule ();
}

void
Neighbors::ScheduleTimer ()
{
  std::cout << "ScheduleTimer" << std::endl;
  m_ntimer.Cancel ();
  m_ntimer.Schedule ();
}

void
Neighbors::AddArpCache (Ptr<ArpCache> a)
{
  std::cout << "AddArpCache" << std::endl;
  m_arp.push_back (a);
}

void
Neighbors::DelArpCache (Ptr<ArpCache> a)
{
  std::cout << "DelArpCache" << std::endl;
  m_arp.erase (std::remove (m_arp.begin (), m_arp.end (), a), m_arp.end ());
}

Mac48Address
Neighbors::LookupMacAddress (Ipv4Address addr)
{
  std::cout << "LookupMacAddress" << std::endl;
  Mac48Address hwaddr;
  for (std::vector<Ptr<ArpCache> >::const_iterator i = m_arp.begin ();
       i != m_arp.end (); ++i)
    {
      ArpCache::Entry * entry = (*i)->Lookup (addr);
      if (entry != 0 && (entry->IsAlive () || entry->IsPermanent ()) && !entry->IsExpired ())
        {
          hwaddr = Mac48Address::ConvertFrom (entry->GetMacAddress ());
          break;
        }
    }
  return hwaddr;
}

void
Neighbors::ProcessTxError (WifiMacHeader const & hdr)
{
  std::cout << "ProcessTxError" << std::endl;
  Mac48Address addr = hdr.GetAddr1 ();

  for (std::vector<Neighbor>::iterator i = m_nb.begin (); i != m_nb.end (); ++i)
    {
      if (i->m_hardwareAddress == addr)
        {
          i->close = true;
        }
    }
  Purge ();
}

}  // namespace aodv
}  // namespace ns3

