#include "ui/viewlayout.hpp"

#include <atomic>

class Installer : public ViewLayout
{
public:
    Installer();

    const char* getName() const override;

    void onRenderUI() override;

private:
    std::atomic<bool> bFinnish;

    void doInBackground();

    bool download_data();
};

