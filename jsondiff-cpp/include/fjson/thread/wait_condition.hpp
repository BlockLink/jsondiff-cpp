#pragma once
#include <fjson/thread/future.hpp>
#include <fjson/thread/spin_yield_lock.hpp>
#include <fjson/thread/unique_lock.hpp>
#include <deque>

namespace fjson
{
  /**
   *  A thread-safe, fiber-aware condition variable that
   *  can be used to signal/wait on a certain condition between
   *  threads.
   */
  template<typename T=void_t>
  class wait_condition
  {
     public:
        wait_condition(const char* name) : _name(name) {}

        void wait( const microseconds& timeout = microseconds::maximum() )
        {
          typename fjson::promise<T>::ptr p = new fjson::promise<T>(_name);
          { synchronized( _prom_lock ) 
            _promises.push_back( p );
          }
          p->wait(timeout);
        }

        template<typename LockType>
        T wait( LockType& l, const microseconds& timeout = microseconds::maximum() )
        {
          typename fjson::promise<T>::ptr p( new fjson::promise<T>(_name));
          { synchronized( _prom_lock ) 
            _promises.push_back( p );
          }
          l.unlock();
          struct relocker {
             LockType& _lock;
             relocker(LockType& l) : _lock(l) {}
             ~relocker() { _lock.lock(); }
          } lock_on_exit(l);
          return p->wait(timeout);
        }

        void notify_one( const T& t=T())
        {
          typename fjson::promise<void_t>::ptr prom;
          { synchronized( _prom_lock ) 
             if( _promises.size() )
             {
                prom = _promises.front();
                _promises.pop_front();
             }
          }

          if( prom && prom->retain_count() > 1 ) 
             prom->set_value(t);
        }
        void notify_all(const T& t=T())
        {
            std::deque<typename fjson::promise<T>::ptr> all;
            { synchronized( _prom_lock ) 
              std::swap(all, _promises);
            }
            for( auto itr = all.begin(); itr != all.end(); ++itr )
            {
              if( (*itr)->retain_count() > 1 ) 
                 (*itr)->set_value(t);
            }
        }
     private:
        fjson::spin_yield_lock                      _prom_lock;
        std::deque<typename fjson::promise<T>::ptr> _promises;
        const char *const                        _name;
  };
}
