#ifndef SIGNALS_H
#define SIGNALS_H

#include <utility>

/** Interface for delegates with a specific set of arguments **/
template<typename... args>
class AbstractDelegate
{
  public:
    virtual void operator()(args...) const = 0;
    virtual ~AbstractDelegate() {}
};

/** Concrete member function delegate that discards the function's return value **/
template<typename T, typename ReturnType, typename... args>
class ObjDelegate : public AbstractDelegate<args...>
{
  public:
    /** member function typedef **/
    using ObjMemFn = ReturnType (T::*)(args...);

    /** constructor **/
    ObjDelegate(T& obj, ObjMemFn memFn)
      : obj_(obj), // brace-enclosed initializer list didn't work here
      memFn_{memFn} // here the brace-enclosed list works, probably because memFn is _not_ a reference
    {
    }

    /** call operator that calls the stored function on the stored object **/
    void operator()(args... a) const override
    {
      (obj_.*memFn_)(std::forward<args>(a)...);
    }

  private:
    /** reference to the object **/
    T& obj_;
    /** member function pointer **/
    const ObjMemFn memFn_;
};

/** Concrete function delegate that discards the function's return value **/
template<typename ReturnType, typename... args>
class FnDelegate : public AbstractDelegate<args...>
{
  public:
    /** member function typedef **/
    using Fn = ReturnType(*)(args...);

    /** constructor **/
    FnDelegate(Fn fn)
      : fn_{fn}
    {
    }

    /** call operator that calls the stored function **/
    void operator()(args... a) const override
    {
      (*fn_)(std::forward<args>(a)...);
    }

  private:
    /** function pointer **/
    const Fn fn_;
};

/** forward declaration **/
template<typename... args>
class Connection;

/** Signal class that can be connected to**/
template<typename... args>
class Signal
{
  public:
    /** connection pointer typedef **/
    using connection_p = Connection<args...>*;

    /** constructor **/
    Signal()
      : connections_(nullptr),
      blocked_(false)
      {
      }

    /** copy constructor **/
    Signal(const Signal& other)
      : connections_(nullptr),
      blocked_(other.blocked()) // not sure if this is a good idea
      {
      }

    /** call operator that notifes all connections associated with this Signal.
      The most recently associated connection will be notified first **/
    void operator()(args... a) const
    {
      // only notify connections if this signal is not blocked
      if (!blocked())
      {
        auto c = connections_;
        while(c)
        {
          auto c_next = c->next();
          if (c_next)
            (*c)(a...);
          else
            (*c)(std::forward<args>(a)...); // last use, can forward
          c = c_next;
        }
      }
    }

    /** connect to this signal **/
    void connect(connection_p p)
    {
      p->next_ = connections_;
      connections_ = p;
      p->signal_ = this;
    }

    /** disconnect from this signal.
      Invalidates the connection's signal pointer
      and removes the connection from the list **/
    void disconnect(connection_p conn)
    {
      // find connection and remove it from the list
      connection_p c = connections_;
      if (c == conn)
      {
        connections_ = connections_->next();
        conn->next_ = nullptr;
        conn->signal_ = nullptr;
        return;
      }
      while(c != nullptr)
      {
        if (c->next() == conn)
        {
          c->next_ = conn->next();
          conn->next_ = nullptr;
          conn->signal_ = nullptr;
          return;
        }
        c = c->next();
      }
    }

    /** block events from this signal **/
    void block()
    {
      blocked_ = true;
    }

    /** unblock events from this signal **/
    void unblock()
    {
      blocked_ = false;
    }

    /** is this signal blocked? **/
    bool blocked() const
    {
      return blocked_;
    }

    /** destructor. disconnects all connections **/
    ~Signal()
    {
      connection_p p = connections_;
      while(p != nullptr)
      {
        connection_p n = p->next();
        disconnect(p);
        p = n;
      }
    }

    connection_p connections() const {return connections_;}

  private:
    /** don't allow copy assignment **/
    Signal& operator= (Signal& other);

    connection_p connections_;
    bool blocked_;
};

/** connection class that can be connected to a signal **/
template<typename... args>
class Connection
{
  public:
    /** template constructor for non-static member functions.
      allocates a new delegate on the heap **/
    template<typename T, typename ReturnType>
    Connection(Signal<args...>& signal, T& obj, ReturnType (T::*memFn)(args...))
      : delegate_(new ObjDelegate<T, ReturnType, args...>(obj, memFn)),
      signal_(nullptr),
      next_(nullptr),
      blocked_(false)
    {
      signal.connect(this);
    }

    /** template constructor for static member functions and free functions.
      allocates a new delegate on the heap **/
    template<typename ReturnType>
    Connection(Signal<args...>& signal, ReturnType (*Fn)(args...))
      : delegate_(new FnDelegate<ReturnType, args...>(Fn)),
      signal_(nullptr),
      next_(nullptr),
      blocked_(false)
    {
      signal.connect(this);
    }

    /** get reference to this connection's delegate **/
    AbstractDelegate<args...>& delegate() const
    {
      return *delegate_;
    }

    /** call this connection's delegate if not blocked **/
    void operator()(args... a) const
    {
      if (!blocked())
      {
        delegate()(std::forward<args>(a)...);
      }
    }

    /** get pointer to next connection in the signal's list **/
    Connection* next() const
    {
      return next_;
    }

    /** is this connection connected to a valid signal? **/
    bool connected() const
    {
      return (signal_ != nullptr);
    }

    /** block events for this connection **/
    void block()
    {
      blocked_ = true;
    }

    /** unblock events for this connection **/
    void unblock()
    {
      blocked_ = false;
    }

    /** is this connection blocked? **/
    bool blocked() const
    {
      return blocked_;
    }

    /** desctructor. If the signal is still alive, disconnects from it **/
    ~Connection()
    {
      if (signal_ != nullptr)
      {
        signal_->disconnect(this);
      }
      delete delegate_;
    }

    const Signal<args...>* signal() const {return signal_;}

    friend class Signal<args...>;
  private:
    /** don't allow copy construction **/
    Connection(const Connection& other);

    /** don't allow copy assignment **/
    Connection& operator= (Connection& other);

    AbstractDelegate<args...>* delegate_;
    Signal<args...>* signal_;
    Connection* next_;
    bool blocked_;
};

/** free connect function: creates a connection (non-static member function) on the heap
  that can be used anonymously **/
template<typename T, typename ReturnType, typename... args>
Connection<args...>* connect(Signal<args...>& signal, T& obj, ReturnType (T::*memFn)(args...))
{
  return new Connection<args...>(signal, obj, memFn);
}

/** free connect function: creates a connection (static member or free function) on the heap
  that can be used anonymously **/
template<typename ReturnType, typename... args>
Connection<args...>* connect(Signal<args...>& signal, ReturnType (*fn)(args...))
{
  return new Connection<args...>(signal, fn);
}

#endif // SIGNALS_H


