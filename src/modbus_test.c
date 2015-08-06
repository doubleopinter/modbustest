
#include </usr/local/beta/include/modbus/modbus.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>

enum {
    TCP,
    TCP_PI,
    RTU
};

int sqlstuff(void)
{
	sqlite3 *db;
	int rc;
	char *sql;

	//sql = "select * from pointdb";
	sql = "INSERT INTO pointdb (PN,Value,Timestamp) VALUES (40001, 34, '03082015 01:11:11'); ";

	printf("Opening DB\n");
	rc = sqlite3_open("/home/pinter/build/test.db", &db);
	printf("Result %d\n", rc);
	printf("Time to write something\n");
	rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
	printf("Result %d\n", rc);
	printf("Close DB");
	sqlite3_close(db);

	return 0;
}

int mmodbus(void)
{
	int a;
	//int backend = TCP;
	int connection;
	modbus_t *ctx;
	//int timeout;
//	uint32_t old_response_to_sec;
//	uint32_t old_response_to_usec;
	uint16_t holding_read[128];
	int read_result;
	FILE *fp;
	time_t rawtime;
	struct tm * timeinfo;
	int i;



	//printf ( "Current local time and date: %s", asctime (timeinfo) );



	ctx = modbus_new_tcp("192.168.10.53", 502);
	modbus_set_slave(ctx, 1);

	/* Save original timeout */
	//modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);

	//printf("Seconds: %d %s %d\n", old_response_to_usec, " uSeconds:", old_response_to_sec, "\n");
//	modbus_set_response_timeout(ctx, 0, 200);

	if (ctx == NULL) {
	    fprintf(stderr, "Unable to allocate libmodbus context\n");
	    return -1;
	}

	/*if (modbus_connect(ctx) == -1) {
	    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
	    printf("Not Connected");
	    modbus_free(ctx);
	    return -1;
	}*/

	connection = modbus_connect(ctx);
	printf("Connected\n");

	//printf("Reading register 40001\n");
	fp = fopen("/tmp/modbus.txt", "a");
	a = 10;
	while( a!=0 ) //Ascii code for ctrl + A == 1
	    {
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );
	        read_result = modbus_read_registers(ctx, 0, 10, holding_read);
	        for (i = 10; i > 0; i--) {


	        	printf("4000%d: %d %s %s", i, holding_read[i], "Time: ", asctime (timeinfo));
	        }
	        //Print contents to file

	        //fprintf(fp, "40001: %d %s %s", holding_read[0], "Time: ", asctime (timeinfo));
	        //fflush(fp);
	        a--;
	        sleep(1);
	    }
	fclose(fp);
	modbus_close(ctx);
	modbus_free(ctx);
	return 0;
}

int main(void)
{
	//sqlstuff();
	mmodbus();
	return 0;
}
