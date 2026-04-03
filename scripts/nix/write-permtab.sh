#!/usr/bin/env bash
set -euo pipefail

sshpass -p "$BOARD_PASS" ssh -n -o StrictHostKeyChecking=no "$BOARD_USER@$BOARD_HOST" "
set -e
cat > /tmp/umdp.permtab <<'EOF'
/root/shared/compiled/blink none none none
/root/shared/compiled/blink_umdp none 0x03020000-0x03021000 none
/root/shared/compiled/uptime none 0x5026000-0x5027000 none
/root/shared/compiled/timer_wait 14 none 0x40-0x44
/root/shared/compiled/blink_timer 14 0x03020000-0x03021000 0x40-0x44
EOF
cat /tmp/umdp.permtab > /proc/umdp/permtab
rm -f /tmp/umdp.permtab
"
