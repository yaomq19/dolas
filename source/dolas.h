#ifndef DOLAS_H
#define DOLAS_H
class Dolas
{
public:
    Dolas();
    ~Dolas();

    void initialize();
    void run();
    void destroy();

    bool isInitialized() const;
private:
    bool m_initialized;
};

#endif