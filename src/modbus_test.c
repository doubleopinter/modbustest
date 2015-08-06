
#include </usr/local/beta/include/modbus/modbus.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>
#include <pthread.h>

enum {
    TCP,
    TCP_PI,
    RTU
};

int sqlstuff(int pt, int typ, int val, char *ts, int rtu, int pro)
{
	sqlite3 *db;
	int rc;
	char *sql;
	char *insert_sql = "INSERT INTO history (PNTNO, TYPE, VALUE ,TIMESTAMP, RTUNO, PROTOCOL) VALUES (?,?,?,?,?,?)";
	sqlite3_stmt *stmt;
	char message[255];
	int temp = 999;

	//sql = "select * from pointdb";
	//sql = "INSERT INTO pointdb (PN,Value,Timestamp) VALUES (40001, 34, '03082015 01:11:11'); ";

	printf("Opening DB\n");
	rc = sqlite3_open("/home/pinter/build/test.db", &db);
	if (rc != SQLITE_OK) {
		printf("Failed to open database %s\n\r",sqlite3_errstr(rc)) ;
	    sqlite3_close(db) ;
	    return 1;
	}
	printf("Opened db OK\n\r") ;

	/* prepare the sql, leave stmt ready for loop */
	rc = sqlite3_prepare_v2(db, insert_sql, strlen(insert_sql)+1, &stmt, NULL) ;
	if (rc != SQLITE_OK) {
		printf("Failed to prepare database %s\n\r",sqlite3_errstr(rc)) ;
		sqlite3_close(db) ;
		return 2;
	}
	printf("SQL prepared ok\n\r") ;

	/* bind parameters */
		sqlite3_bind_int(stmt, 1, pt); /* index 1 in stmt - point number */
		sqlite3_bind_int(stmt, 2, typ); /* Type */
		sqlite3_bind_int(stmt, 3, val); /* Value */
		sqlite3_bind_text(stmt, 4, ts, strlen(ts), SQLITE_STATIC);/* Time*/
		sqlite3_bind_int(stmt,5, rtu); /* RTUNO */
		sqlite3_bind_int(stmt, 6, pro); /* Protocol */


		rc = sqlite3_step(stmt);

	    /* finish off */
		sqlite3_close(db);
/*8
	printf("Result %d\n", rc);
	printf("Time to write something\n");
	rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
	printf("Result %d\n", rc);
	printf("Close DB");
	sqlite3_close(db);
*/
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
	pthread_t id = pthread_self();

	//SQL connection
	sqlite3 *db;
	int rc;
	char *sql;

	rc = sqlite3_open("/home/pinter/build/test.db", &db);
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
	//fp = fopen("/tmp/modbus.txt", "a");
	//a = 10;
	while(1) //Ascii code for ctrl + A == 1
	    {
			time ( &rawtime );
			timeinfo = localtime ( &rawtime );
	        read_result = modbus_read_registers(ctx, 0, 10, holding_read);
	        for (i = 8; i > -1; i--) {
	        	sqlstuff(i, 1, holding_read[i], asctime(timeinfo), 1, 1);
	        	printf("4000%d: %d %s %s", i, holding_read[i], "Time: ", asctime (timeinfo));
	        }
	        //Print contents to file

	        //fprintf(fp, "40001: %d %s %s", holding_read[0], "Time: ", asctime (timeinfo));
	        //fflush(fp);
	        sleep(1);
	    }
	//
	//fclose(fp);
	modbus_close(ctx);
	modbus_free(ctx);
	return 0;
}

int main(void)
{
	pthread_t tid;

	//sqlstuff();

	pthread_create(&tid, NULL, &mmodbus, NULL);

	printf ( "Press [Enter] to continue . . ." );
	fflush ( stdout );
	getchar();
	printf("Testicles");

	return 0;
}
