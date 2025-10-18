#ifndef PGBAR_INDICATOR
#define PGBAR_INDICATOR

#include "details/concurrent/Util.hpp"
#include "details/console/TermContext.hpp"
#include "details/render/Renderer.hpp"

namespace pgbar {
  namespace config {
    void hide_completed( bool flag ) noexcept;
    __PGBAR_NODISCARD bool hide_completed() noexcept;
    void disable_styling( bool flag ) noexcept;
    __PGBAR_NODISCARD bool disable_styling() noexcept;
  }

  class Indicator {
    static std::atomic<bool> _hide_completed;
    static std::atomic<bool> _disable_styling;

    friend void config::hide_completed( bool ) noexcept;
    friend bool config::hide_completed() noexcept;
    friend void config::disable_styling( bool ) noexcept;
    friend bool config::disable_styling() noexcept;

  public:
    Indicator()                                = default;
    Indicator( const Indicator& )              = delete;
    Indicator& operator=( const Indicator& ) & = delete;
    Indicator( Indicator&& )                   = default;
    Indicator& operator=( Indicator&& ) &      = default;
    virtual ~Indicator()                       = default;

    virtual void reset()                                   = 0;
    virtual void abort() noexcept                          = 0;
    __PGBAR_NODISCARD virtual bool active() const noexcept = 0;

    // Wait until the indicator is Stop.
    void wait() const noexcept
    {
      __details::concurrent::spin_wait( [this]() noexcept { return !active(); } );
    }
    // Wait for the indicator is Stop or timed out.
    template<class Rep, class Period>
    __PGBAR_NODISCARD bool wait_for( const std::chrono::duration<Rep, Period>& timeout ) const noexcept
    {
      return __details::concurrent::spin_wait_for( [this]() noexcept { return !active(); }, timeout );
    }
  };
  __PGBAR_CXX17_INLINE std::atomic<bool> Indicator::_hide_completed { false };
  __PGBAR_CXX17_INLINE std::atomic<bool> Indicator::_disable_styling { true };

  namespace config {
    inline void hide_completed( bool flag ) noexcept
    {
      Indicator::_hide_completed.store( flag, std::memory_order_relaxed );
    }
    __PGBAR_NODISCARD inline bool hide_completed() noexcept
    {
      return Indicator::_hide_completed.load( std::memory_order_relaxed );
    }
    /**
     * Whether to automatically disable the style effect of the configuration object
     * when the output stream is not directed to a terminal.
     */
    inline void disable_styling( bool flag ) noexcept
    {
      Indicator::_disable_styling.store( flag, std::memory_order_relaxed );
    }
    __PGBAR_NODISCARD inline bool disable_styling() noexcept
    {
      return Indicator::_disable_styling.load( std::memory_order_relaxed );
    }

    /**
     * Determine if the output stream is binded to the tty based on the platform api.
     *
     * Always returns true if defined `PGBAR_INTTY`,
     * or the local platform is neither `Windows` nor `unix-like`.
     */
    __PGBAR_NODISCARD inline bool intty( Channel channel ) noexcept
    {
      if ( channel == Channel::Stdout )
        return __details::console::TermContext<Channel::Stdout>::itself().detect();
      return __details::console::TermContext<Channel::Stderr>::itself().detect();
    }

    __PGBAR_NODISCARD inline __details::types::Size terminal_width( Channel channel ) noexcept
    {
      if ( channel == Channel::Stdout )
        return __details::console::TermContext<Channel::Stdout>::itself().width();
      return __details::console::TermContext<Channel::Stderr>::itself().width();
    }

    using TimeUnit = __details::types::TimeUnit;
    // Get the current output interval.
    template<Channel Outlet>
    __PGBAR_NODISCARD TimeUnit refresh_interval() noexcept
    {
      return __details::render::Renderer<Outlet, Policy::Async>::working_interval();
    }
    // Set the new output interval.
    template<Channel Outlet>
    void refresh_interval( TimeUnit new_rate ) noexcept
    {
      __details::render::Renderer<Outlet, Policy::Async>::working_interval( new_rate );
    }
    // Set every channels to the same output interval.
    inline void refresh_interval( TimeUnit new_rate ) noexcept
    {
      __details::render::Renderer<Channel::Stderr, Policy::Async>::working_interval( new_rate );
      __details::render::Renderer<Channel::Stdout, Policy::Async>::working_interval( new_rate );
    }
  } // namespace config
} // namespace pgbar

#endif
