#include "ui/viewlayout.hpp"

#include <atomic>

class Installer : public ViewLayout
{
public:
    Installer();

    void onRender() override;

private:
    std::atomic<bool> bFinnish;

    void doInBackground();

    bool download_data();

    static void DoInForegroundFinnishInstaller(ViewLayout* view);
};

