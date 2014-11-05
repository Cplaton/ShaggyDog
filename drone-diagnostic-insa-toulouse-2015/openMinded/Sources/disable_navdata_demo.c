#include <unistd.h>
#include <ardrone_api.h>
#include <ardrone_tool/UI/ardrone_input.h>

#include <ardrone_tool/ardrone_tool_configuration.h>


DEFINE_THREAD_ROUTINE(th_dis_navdata_demo, data)
{

    bool_t value = FALSE;

    usleep(5000000);
    ARDRONE_TOOL_CONFIGURATION_ADDEVENT(navdata_demo, &value, NULL);


    return (0);
}

