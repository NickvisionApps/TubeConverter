#ifndef CONTROLPTR_H
#define CONTROLPTR_H

#include <adwaita.h>
#include "controlbase.h"

namespace Nickvision::TubeConverter::GNOME::Helpers
{
    /**
     * @brief A pointer for a custom gtk control.
     * @brief Keeps the pointer alive until the control has been destroyed.
     */
    template<typename T>
    class ControlPtr
    {
    public:
        /**
         * @brief Constructs a ControlPtr.
         */
        ControlPtr()
            : m_ptr{ nullptr }
        {

        }
        /**
         * @brief Constructs a ControlPtr.
         * @param args the arguments to pass to T's constructor.
         */
        template<typename... Args>
        ControlPtr(Args... args)
            : m_ptr{ new T(args...) }
        {
            g_signal_connect(m_ptr->gobj(), "destroy", GCallback(+[](GtkWidget*, gpointer data){ delete reinterpret_cast<T*>(data); }), m_ptr);
        }
        /**
         * @brief Constructs a ControlPtr.
         * @brief The ControlPtr assumes ownership of ptr.
         * @param ptr T*
         */
        ControlPtr(T* ptr)
            : m_ptr{ ptr }
        {
            g_signal_connect(m_ptr->gobj(), "destroy", GCallback(+[](GtkWidget*, gpointer data){ delete reinterpret_cast<T*>(data); }), m_ptr);
        }
        /**
         * @brief Constructs a ControlPtr via copy.
         * @param other The ControlPtr to copy
         */
        ControlPtr(const ControlPtr& other)
            : m_ptr{ other.m_ptr }
        {

        }
        /**
         * @brief Constructs a ControlPtr via move.
         * @param other The ControlPtr to move
         */
        ControlPtr(ControlPtr&& other)
            : m_ptr{ other.m_ptr }
        {

        }
        /**
         * @brief Gets whether or not the ControlPtr is valid.
         * @return True if valid, else false
         */
        bool isValid() const
        {
            return m_ptr != nullptr;
        }
        /**
         * @brief Returns the underlying pointer.
         * @return T*
         */
        T* operator->() const
        {
            return m_ptr;
        }
        /**
         * @brief Assigns a ControlPtr via copy.
         * @param other The ControlPtr to copy
         * @return this
         */
        ControlPtr& operator=(const ControlPtr& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }
        /**
         * @brief Assigns a ControlPtr via move.
         * @param other The ControlPtr to move
         * @return this
         */
        ControlPtr& operator=(ControlPtr&& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }
        /**
         * @brief Gets whether or not the ControlPtr is valid.
         * @return True if valid, else false
         */
        operator bool() const
        {
            return isValid();
        }

    private:
        T* m_ptr;
    };
}

#endif //CONTROLPTR_H
