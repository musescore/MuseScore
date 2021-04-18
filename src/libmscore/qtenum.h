#ifdef SCRIPT_INTERFACE
#define BEGIN_QT_REGISTERED_ENUM(Name) \
    class MSQE_##Name { \
        Q_GADGET \
    public:
#define END_QT_REGISTERED_ENUM(Name) \
    Q_ENUM(Name); \
}; \
    using Name = MSQE_##Name::Name;
#else
#define BEGIN_QT_REGISTERED_ENUM(Name)
#define END_QT_REGISTERED_ENUM(Name)
#endif
