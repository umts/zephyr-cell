#include <string.h>
#include <zephyr.h>
#include <stdlib.h>
#include <net/socket.h>
#include <net/http_client.h>
#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>
#include <modem/at_cmd.h>
#include <modem/at_notif.h>
#include <modem/modem_key_mgmt.h>

#define SLEEP_TIME_MS 1000
#define HTTP_PORT "80"
#define HTTP_HOST "bustracker.pvta.com"
#define STOP_ID "73"
#define HTTP_PATH "/InfoPoint/rest/stopdepartures/get/" STOP_ID
#define REQUEST "GET " HTTP_PATH " HTTP/1.0\r\nHost: " HTTP_HOST "\r\n\r\n"
#define RECV_BUF_SIZE 24000

#define SSTRLEN(s) (sizeof(s) - 1)
#define CHECK(r) { if (r == -1) { printf("Error: " #r "\n"); exit(1); } }

#define HTTP_HEAD_LEN (sizeof(HTTP_HEAD) - 1)

static char response[RECV_BUF_SIZE];

void dump_addrinfo(const struct addrinfo *ai) {
  printf("addrinfo @%p: ai_family=%d, ai_socktype=%d, ai_protocol=%d, "
	  "sa_family=%d, sin_port=%x\n",
	  ai, ai->ai_family, ai->ai_socktype, ai->ai_protocol,
	  ai->ai_addr->sa_family,
	  ((struct sockaddr_in *)ai->ai_addr)->sin_port);
}

/* Initialize AT communications */
int at_comms_init(void) {
	int err;

	err = at_cmd_init();
	if (err) {
		printk("Failed to initialize AT commands, err %d\n", err);
		return err;
	}

	err = at_notif_init();
	if (err) {
		printk("Failed to initialize AT notifications, err %d\n", err);
		return err;
	}

	return 0;
}

char* http_get_request(void) {
  static struct addrinfo hints;
	struct addrinfo *res;
	int len, err, st, sock;
  size_t off = 0;

  k_msleep(SLEEP_TIME_MS);
  printf("Preparing HTTP GET request for http://" HTTP_HOST ":" HTTP_PORT HTTP_PATH "\n");

	err = nrf_modem_lib_init(NORMAL_MODE);
	if (err) {
		printk("Failed to initialize modem library!");
		return;
	}

	/* Initialize AT comms in order to provision the certificate */
	err = at_comms_init();
	if (err) {
		return;
	}

	printk("Waiting for network.. ");
	err = lte_lc_init_and_connect();
	if (err) {
		printk("Failed to connect to the LTE network, err %d\n", err);
		return;
	}
	printk("OK\n");

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  st = getaddrinfo(HTTP_HOST, HTTP_PORT, &hints, &res);
  printf("getaddrinfo status: %d\n", st);

  if (st != 0) {
    printf("Unable to resolve address, quitting\n");
    return;
  }

  dump_addrinfo(res);
  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  CHECK(sock);
  printf("sock = %d\n", sock);

  CHECK(connect(sock, res->ai_addr, res->ai_addrlen));
	CHECK(send(sock, REQUEST, SSTRLEN(REQUEST), 0));

  printf("Response:\n\n");

  // while (1) {
	// 	int len = recv(sock, response, sizeof(response) - 1, 0);

	// 	if (len < 0) {
	// 		printf("Error reading response\n");
	// 		return;
	// 	}

	// 	if (len == 0) {
	// 		break;
	// 	}

	// 	response[len] = 0;
	// 	printf("%s", response);
	// }

  do {
    int p = poll(sock);
		len = recv(sock, &response[off], RECV_BUF_SIZE - off, 0);
    printk("len size %d\n", len);
		if (len < 0) {
			printk("recv() failed, err %d\n", errno);
			goto clean_up;
		}
		off += len;
	} while (len != 0 /* peer closed connection */);

	printk("Received %d bytes\n", off);

	/* Print HTTP response */
	// char *p = strstr(response, "\r\n\r\n");
	// if (p) {
	// 	off = p - response;
	// 	response[off + 1] = '\0';
	// }

  clean_up:
	  (void)close(sock);

  return(response);
}