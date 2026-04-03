#!/usr/bin/env bash
set -euo pipefail

sshpass -p "$BOARD_PASS" ssh -n -o StrictHostKeyChecking=no "$BOARD_USER@$BOARD_HOST" "
set -e
killall blink blink_umdp blink_timer timer_wait uptime 2>/dev/null || true
rmmod umdp 2>/dev/null || true
insmod $BOARD_SHARED_DIR/umdp.ko
lsmod | grep '^umdp'

if [ -d /proc/device-tree ]; then
  echo '--- DT snippet (timer/plic) ---'
  ls /proc/device-tree | grep -i plic || true
  ls /proc/device-tree/soc 2>/dev/null | grep -i timer || true
fi

if [ -d /sys/firmware/devicetree/base ]; then
  echo '--- Runtime DTB check ---'
  ls /sys/firmware/devicetree/base/soc 2>/dev/null | grep -i timer || true
fi

cat > /tmp/umdp.permtab <<'EOF'
/root/shared/compiled/blink none none none
/root/shared/compiled/blink_umdp none 0x03020000-0x03021000 none
/root/shared/compiled/uptime none 0x5026000-0x5027000 none
/root/shared/compiled/timer_wait 79 0x030a0000-0x030a1000 none
/root/shared/compiled/blink_timer 79 0x03020000-0x03021000,0x030a0000-0x030a1000 none
EOF
cat /tmp/umdp.permtab > /proc/umdp/permtab
rm -f /tmp/umdp.permtab

echo '=== TEST: uptime ==='
$BOARD_SHARED_DIR/compiled/uptime

echo '=== TEST: blink (sysfs, timeout) ==='
timeout 3 $BOARD_SHARED_DIR/compiled/blink || test \$? -eq 124

echo '=== TEST: blink_umdp (timeout) ==='
timeout 3 $BOARD_SHARED_DIR/compiled/blink_umdp || test \$? -eq 124

echo '=== TEST: timer_wait (timeout) ==='
timeout 4 $BOARD_SHARED_DIR/compiled/timer_wait 250 || test \$? -eq 124

echo '=== TEST: blink_timer (timeout) ==='
timeout 4 $BOARD_SHARED_DIR/compiled/blink_timer 250 || test \$? -eq 124

echo '--- UMDP dmesg ---'
dmesg | grep -i umdp | tail -n 120 || true
echo '--- /proc/interrupts ---'
grep -E '(^ *CPU|\s79:|\s54:|timer|dw|plic)' /proc/interrupts || true
"
