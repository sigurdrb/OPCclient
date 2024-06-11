#include "open62541.c"
#include "open62541.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <time.h>

static volatile int keepRunning = 1;
UA_UInt16 value3 = 60;
float y;
float u = 1;	
float ykm1 = 0;
UA_UInt16 yword;

	

void intHandler(int dummy) {
    keepRunning = 0;
}

/* Function that ramps a tank up and down*/
UA_UInt16 levelSimulation() {
	float y = 0.05*u+ykm1;
	if (y < 0) {
		y = 0;
		u = 1;
	}
	else if (y > 4) {
		y = 4;
		u = -1;
	}
	ykm1 = y;
	yword = round(1029.6576*y);
	if (yword < 1) {
		yword = 1;		
	}
	else if (yword > 4096) {
		yword = 4096;		
	}
	//printf("The function value is: %d\n", yword);		
	return yword;
}



int main(void) {
	signal(SIGINT, intHandler);
	UA_Client *client = UA_Client_new();
	UA_ClientConfig_setDefault(UA_Client_getConfig(client));
	UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://192.168.4.100:4840");
	if(retval != UA_STATUSCODE_GOOD) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
		    "The connection failed with status code %s",
		    UA_StatusCode_name(retval));
	UA_Client_delete(client);
	return 0;
	}

	/* Read the value attribute of the node. UA_Client_readValueAttribute is a 
	* wrapper for the raw read service available as UA_Client_Service_read. */
	UA_Variant value; /* Variants can hold scalar values and arrays of any type */
	UA_Variant_init(&value);

	/* Read attribute */
	UA_UInt16 value1 = 0;
	UA_Variant *val = UA_Variant_new();
	retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(4, "|var|PFC200.Application.GVL_Simulation.tankLevelSim[1]"), val);
	if(retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
	val->type == &UA_TYPES[UA_TYPES_UINT16]) {
	    value1 = *(UA_UInt16*)val->data;
	}
	UA_Variant_delete(val);

	/* Write node attribute (using the highlevel API) */
	UA_UInt16 value2 = 58;
	UA_Variant *myVariant = UA_Variant_new();
	UA_Variant_setScalarCopy(myVariant, &value2, &UA_TYPES[UA_TYPES_UINT16]);

	UA_Variant_delete(myVariant);


	UA_DateTime datetime;
	datetime = time(NULL);	
	//printf("current time: %ld \n", datetime);
	UA_DateTimeStruct time;
	time.year = 2000;
	time.month = 3;
	time.day = 5;

	UA_Variant *myTimeVariant = UA_Variant_new();
	UA_Variant_setScalarCopy(myTimeVariant, &datetime, &UA_TYPES[UA_TYPES_DATETIME]);
	//ua_client_writevalueattribute(client, ua_nodeid_string(4, "|var|PFC200.application.gvl_array.channelinputarray[1]"), myvariant);


	




	/* NodeId of the variable holding the current time */
	const UA_NodeId nodeId =
	UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME);
	retval = UA_Client_readValueAttribute(client, nodeId, &value);

	if(retval == UA_STATUSCODE_GOOD &&
	UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DATETIME])) {
	UA_DateTime raw_date = *(UA_DateTime *) value.data;
	UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
		    "date is: %u-%u-%u %u:%u:%u.%03u",
		    dts.day, dts.month, dts.year, dts.hour,
		    dts.min, dts.sec, dts.milliSec);
	} else {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
		    "Reading the value failed with status code %s",
		    UA_StatusCode_name(retval));
	}


	while(keepRunning) {	
		UA_Variant *myVariant = UA_Variant_new();
		value3 = levelSimulation();
		UA_Variant_setScalarCopy(myVariant, &value3, &UA_TYPES[UA_TYPES_UINT16]);
		UA_Client_writeValueAttribute(client, UA_NODEID_STRING(4, "|var|PFC200.Application.GVL_Simulation.tankLevelSim[1]"), myVariant);
		UA_Variant_delete(myVariant);	
		value3++;
		//printf("The value is: %d\n", value3);
		//printf("The test value is: %d\n", levelSimulation());
		usleep(100000);
		}
	//printf("The end: %d\n", value3);
	/* Clean up */
	UA_Variant_clear(&value);
	UA_Client_delete(client); /* Disconnects the client internally */
    return 0;
}
