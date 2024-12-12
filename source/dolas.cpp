#include "dolas.h"
Dolas::Dolas()
{
    m_initialized = false;
}

Dolas::~Dolas()
{
}

void Dolas::initialize()
{
    m_initialized = true;
}

void Dolas::run()
{
    while (true)
    {

    }
}

void Dolas::destroy()
{
    m_initialized = false;
}

bool Dolas::isInitialized() const
{
    return m_initialized;
}