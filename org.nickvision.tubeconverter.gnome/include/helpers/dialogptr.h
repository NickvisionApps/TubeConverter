#ifndef DIALOGPTR_H
#define DIALOGPTR_H

#include <adwaita.h>
#include "dialogbase.h"

namespace Nickvision::TubeConverter::GNOME::Helpers
{
    template<typename T>
    concept DerivedDialogBase = std::is_base_of_v<DialogBase, T>;

    /**
     * @brief A pointer for a custom AdwDialog.
     * @brief Keeps the pointer alive until the dialog has closed.
     */
    template<DerivedDialogBase T>
    class DialogPtr
    {
    public:
        /**
         * @brief Constructs a DialogPtr.
         */
        DialogPtr()
            : m_ptr{ nullptr }
        {

        }
        /**
         * @brief Constructs a DialogPtr.
         * @param args The arguments to pass to T's constructor.
         */
        template<typename... Args>
        DialogPtr(Args... args)
            : m_ptr{ new T(args...) }
        {
            g_signal_connect(m_ptr->gobj(), "closed", GCallback(+[](AdwDialog*, gpointer data)
            { 
                T* ptr{ reinterpret_cast<T*>(data) };
                ptr->closed().invoke({});
                delete ptr; 
            }), m_ptr);
        }
        /**
         * @brief Constructs a DialogPtr.
         * @brief The DialogPtr assumes ownership of ptr.
         * @param ptr T*
         */
        DialogPtr(T* ptr)
            : m_ptr{ ptr }
        {
            g_signal_connect(m_ptr->gobj(), "closed", GCallback(+[](AdwDialog*, gpointer data)
            { 
                T* ptr{ reinterpret_cast<T*>(data) };
                ptr->closed().invoke({});
                delete ptr; 
            }), m_ptr);
        }
        /**
         * @brief Constructs a DialogPtr via copy.
         * @param other The DialogPtr to copy
         */
        DialogPtr(const DialogPtr& other)
            : m_ptr{ other.m_ptr }
        {

        }
        /**
         * @brief Constructs a DialogPtr via move.
         * @param other The DialogPtr to move
         */
        DialogPtr(DialogPtr&& other)
            : m_ptr{ other.m_ptr }
        {

        }
        /**
         * @brief Gets whether or not the DialogPtr is valid.
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
         * @brief Assigns a DialogPtr via copy.
         * @param other The DialogPtr to copy
         * @return this
         */
        DialogPtr& operator=(const DialogPtr& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }
        /**
         * @brief Assigns a DialogPtr via move.
         * @param other The DialogPtr to move
         * @return this
         */
        DialogPtr& operator=(DialogPtr&& other)
        {
            m_ptr = other.m_ptr;
            return *this;
        }
        /**
         * @brief Gets whether or not the DialogPtr is valid.
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

#endif //DIALOGPTR_H
