
#include </usr/local/beta/include/modbus/modbus.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>
#include <pthread.h>

#define ANALOG_IN 1

enum {
    TCP,
    TCP_PI,
    RTU
};

struct arg_struct {
    char *ip;
    int interval;
    int start_point;
    int length;
};

int sqlstuff(int start_pt, int length, int typ, uint16_t *val, char *ts, int rtu, int pro)
{
	sqlite3 *db;
	int rc; /* SQL transaction result code */
	char *insert_sql = "INSERT INTO history (PNTNO, TYPE, VALUE ,TIMESTAMP, RTUNO, PROTOCOL) VALUES (?,?,?,?,?,?)";
	sqlite3_stmt *stmt;
	int i;

	//Open the connection to database
	printf("Opening DB\n");
	rc = sqlite3_open("/home/pinter/build/test.db", &db);
	if (rc != SQLITE_OK) {
		printf("Failed to open database %s\n\r",sqlite3_errstr(rc)) ;
	    sqlite3_close(db) ;
	    return 1;
	}
	printf("Opened db OK\n\r") ;

	/* prepare the sql, leave stmt ready for loop */


	for (i = 0; i < length; i++) {
		rc = sqlite3_prepare_v2(db, insert_sql, strlen(insert_sql)+1, &stmt, NULL) ;
		if (rc != SQLITE_OK) {
			printf("Failed to prepare database %s\n\r",sqlite3_errstr(rc)) ;
			sqlite3_close(db) ;
			return 2;
		}
		printf("SQL prepared ok\n\r") ;

		/* bind parameters */
		printf("Array Val: %d\n", val[0]);
		sqlite3_bind_int(stmt, 1, start_pt + i); /* index 1 in stmt - point number */
		sqlite3_bind_int(stmt, 2, typ); /* Type */
		sqlite3_bind_int(stmt, 3, val[i]); /* Value */
		sqlite3_bind_text(stmt, 4, ts, strlen(ts), SQLITE_STATIC);/* Time*/
		sqlite3_bind_int(stmt,5, rtu); /* RTUNO */
		sqlite3_bind_int(stmt, 6, pro); /* Protocol */


		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE) {
			printf("Failed to step %s\n\r",sqlite3_errstr(rc)) ;
			printf("Breaking from write operation\n");
			break;
			//return 2;
		}
		rc = sqlite3_finalize(stmt);
	}
	printf("SQL write complete\n");
	    /* finish off */
	sqlite3_close(db);
	return 0;
}

//Function for modbus holidng register polling
void modbus_holding_poll(void *arguments)
{
	struct arg_struct *args = arguments;
	modbus_t *ctx;
	uint16_t holding_read[128];
	int read_result;
	//FILE *fp;
	time_t rawtime;
	struct tm * timeinfo;
	int i;

	ctx = modbus_new_tcp(args->ip, 502);
	modbus_set_slave(ctx, 1);

	/* Save original timeout */
	//modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);

	//printf("Seconds: %d %s %d\n", old_response_to_usec, " uSeconds:", old_response_to_sec, "\n");
	//modbus_set_response_timeout(ctx, 0, 200);

	if (ctx == NULL) {
	    fprintf(stderr, "Unable to allocate libmodbus context\n");
	    //return -1;
	}

	if (modbus_connect(ctx) == -1) {
	    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
	    printf("Not Connected");
	    modbus_free(ctx);
	    //return -1;
	}
	printf("Connected\n");

	while(1) {
		//Get current system time
		time(&rawtime);
		timeinfo = localtime(&rawtime);

		//Perform the modbus scan
		read_result = modbus_read_registers(ctx, args->start_point, args->length, holding_read);
		if (read_result == -1) {
		    printf("MODBUS polling error, program mad!\n");
			fprintf(stderr, "%s\n", modbus_strerror(errno));
		    break;
		}
        sqlstuff(args->start_point, args->length, ANALOG_IN, holding_read, asctime(timeinfo), 1, 1);

        if (args->interval <= 1) {
        	sleep(1);
	    } else if (args->interval > 10) {
	    	sleep(10);
	    } else {
	    	sleep(args->interval);
	    }
	}

	modbus_close(ctx);
	modbus_free(ctx);
}

int main(void)
{
	pthread_t tid;
	struct arg_struct args;

	args.ip = "192.168.10.53";
	args.start_point = 0;
	args.length = 5;
	args.interval = 1;

	pthread_create(&tid, NULL, &modbus_holding_poll, (void *)&args);

	printf("Press [Enter] to continue . . .");
	fflush(stdout);
	getchar();
	printf("Testicles");

	return 0;
}
