namespace KDDockWidgets {
class Config;
class DockRegistry;
class DragController;

class ContextData
{
public:

    static ContextData* context(int ctx);
    static void destroyContext(int ctx);
    static int contextCount();

    Config* config = nullptr;
    DockRegistry* reg = nullptr;
    DragController* dctrl = nullptr;
};
}
