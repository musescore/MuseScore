
function main()
{
    api.log.info("----------- begin script simple 1 ---------------")

    api.autobot.setInterval(1000)

    api.autobot.openProject("simple1.mscz");
    api.autobot.sleep()

    api.dispatcher.dispatch("zoom-x-percent", [100]);
    api.autobot.sleep()

    api.dispatcher.dispatch("zoom-x-percent", [50]);
    api.autobot.sleep()

    api.dispatcher.dispatch("zoom-x-percent", [100]);
    api.autobot.sleep()

    api.log.info("----------- end script simple 1 ---------------")
}
