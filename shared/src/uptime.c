/*
 * SG2000 RTC Uptime Reader using UMDP
 * Displays the raw RTC seconds counter.
 *
 * On this image, the RTC block is not enabled by default at boot, so this
 * program calls rtc_ensure_running(..., 0) to enable it without seeding a
 * fake date. The reported value is the elapsed time since the counter was
 * enabled (often first run after power cycle).
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "umdp.h"
#include "rtc/rtc.h"

int main(void)
{
  umdp_connection *conn;
  rtc_ctx rtc;
  int ret;

  printf("=== SG2000 RTC Counter Reader ===\n");

  /* 1. Connect to UMDP */
  conn = umdp_connect();
  if (conn == NULL)
  {
    fprintf(stderr, "Failed to connect to UMDP\n");
    return 1;
  }

  /* 2. Map RTC registers into user space */
  ret = rtc_init(&rtc, conn);
  if (ret != 0)
  {
    fprintf(stderr, "Failed to initialize RTC library: %s\n", umdp_strerror(ret));
    umdp_disconnect(conn);
    return 1;
  }
  printf("RTC physically mapped at virtual address: %p\n\n", (void *)rtc.rtc_base);

  /* 3. Ensure RTC block is running (do not seed fallback date) */
  (void)rtc_ensure_running(&rtc, 0u);

  /* 4. Read RTC counter */
  printf("RTC Counter (seconds since RTC counter enable):\n");
  for (int i = 0; i < 3; i++)
  {
    uint32_t rtc_seconds = rtc_read_seconds(&rtc);
    
    uint32_t hours = rtc_seconds / 3600;
    uint32_t minutes = (rtc_seconds % 3600) / 60;
    uint32_t seconds = rtc_seconds % 60;
    
    printf("[%d] %u seconds (%u h %u m %u s)\n", i + 1, rtc_seconds, hours, minutes, seconds);

    if (i < 2)
    {
      sleep(1);
    }
  }

  /* 5. Disconnect */
  umdp_disconnect(conn);
  return 0;
}
