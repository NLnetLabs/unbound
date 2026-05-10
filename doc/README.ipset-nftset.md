## ipset module — populate firewall sets from DNS

The ipset module lets Unbound automatically populate kernel firewall sets
with the IP addresses it resolves. Whenever a DNS query matches a
configured `local-zone "..." ipset` (or `local-zone "..." nftset`), the
resolved A / AAAA records are inserted into the corresponding set so that
firewall rules acting on that set take effect immediately.

The module supports two backends, chosen by the configuration section used:

| Section     | Targets                              | Notes                          |
|-------------|--------------------------------------|--------------------------------|
| `ipset:`    | iptables `ipset` (Linux), PF (BSD)   | Legacy ipset API.              |
| `nftset:`   | nftables sets (Linux only)           | Requires `--enable-nftset`.    |

Both sections share the same options; `nftset:` just adds two extra
fields (`family:` and `table:`). Only one section may be used per Unbound
instance.

---

### Use case

Firewall or routing rules which need to be applied to specific domains, 
rather than static IP address lists.
Unbound can populate the firewall ip address set each time a new IP is
resolved for the target domain.

---

### Build

```
# ipset backend only (iptables ipset / BSD PF):
./configure --enable-ipset

# nftset backend only (nftables, Linux only):
./configure --enable-nftset

# both backends (pick at runtime via section name):
./configure --enable-ipset --enable-nftset

make && make install
```

Either flag may be used on its own, or both together. On Linux either
backend uses `libmnl` send netlink directly to the kernel, so the
build needs `libmnl-dev` (or `libmnl-devel`); the nft backend
additionally needs the kernel headers `linux/netfilter.h` and
`linux/netfilter/nf_tables.h`. On BSD only `--enable-ipset` is supported
and this uses `/dev/pf` directly.

---

### Configuration

Enable the module in `module-config:`, mark zones with
`local-zone: "..." ipset`, and put the set details under an `ipset:`
or `nftset:` block:

```
# unbound.conf — ipset backend
server:
    module-config: "ipset validator iterator"
    local-zone: "example.com." ipset
    local-zone: "blocked.org." ipset

ipset:
    name-v4: "blacklist"
    name-v6: "blacklist6"
```

For the nft backend, configure `module-config:` with "nftset", use the
`nftset:` block, and add `family:` (defaults to `inet` if omitted) and
`table:` (required):

```
# unbound.conf — nftset backend
server:
    module-config: "nftset validator iterator"
    local-zone: "example.com." nftset

nftset:
    family: "inet"          # "inet" (default), "ip", or "ip6"
    table: "fw4"
    name-v4: "blacklist"
    name-v6: "blacklist6"
```

#### Per-zone set routing

`set:` entries route individual zones to specific sets. `name-v4` /
`name-v6` then become optional fallbacks for any zones not covered by a
`set:` line; at least one `set:` line *or* a `name-v4` / `name-v6` pair
must be present.

```
server:
    local-zone: "netflix.com." nftset
    local-zone: "nflxso.net."  nftset
    local-zone: "example.com." nftset

nftset:
    table: "fw4"
    set: "netflix.com." "vpn_v4"  "vpn_v6"
    set: "nflxso.net."  "vpn_v4"  "vpn_v6"
    set: "example.com." "work_v4" "work_v6"
```

Each `set:` line takes three arguments: the zone name, the IPv4 set name,
and the IPv6 set name. The zone name may include or omit the trailing
dot. If a zone has a matching `local-zone: "..." ipset` entry but no
`set:` line, it falls back to the global `name-v4` / `name-v6` (if
configured), or is skipped if neither is present.

---

### Creating the sets

**ipset (iptables)**:
```
ipset -N blacklist  iphash
ipset -N blacklist6 iphash
```

**nft (nftables)**:

The nft backend always inserts elements as intervals (a start address and
an exclusive end address one greater), so the destination set **must**
declare `flags interval`. Combine it with `flags timeout` if you also
want elements to age off automatically:

```
nft add table inet fw4
nft add set inet fw4 blacklist  { type ipv4_addr\; flags interval\; }
nft add set inet fw4 blacklist6 { type ipv6_addr\; flags interval\; }
```

Or, in a loaded ruleset file:

```
set blacklist {
    type ipv4_addr
    flags interval, timeout
    timeout 5m   # fallback TTL when none is supplied by the resolver
}
```

A set defined without `flags interval` will reject every insert with
`EOPNOTSUPP` / `EINVAL`; unbound logs the failure once for that
`(table, setname)` and then silently skips further inserts to that
destination for the rest of the process's lifetime.

---

### Firewall rules

**iptables**:
```
iptables  -A INPUT -m set --match-set blacklist  src -j DROP
ip6tables -A INPUT -m set --match-set blacklist6 src -j DROP
```

**nftables**:
```
nft add rule inet fw4 output ip  daddr @blacklist  drop
nft add rule inet fw4 output ip6 daddr @blacklist6 drop
```

---

### Set lifecycle — administrator responsibility

**Unbound does not create or destroy sets.** It only inserts elements.
The sets must already exist when unbound starts, and they must continue
to exist for the lifetime of the unbound process.

If unbound is unable to add to a set (for example because the set does
not exist), it logs the error **once** for that `(table, setname)` pair
and then suppresses further attempts to that destination for the rest of
the process's lifetime. Fix the configuration and reload to retry.

On systems where the firewall configuration is managed by a tool (e.g.
OpenWrt's `fw4`, `firewalld`), the sets must be defined in a way that
survives firewall reloads and reboots — typically a drop-in config file
loaded by the firewall tool. Ad-hoc `nft add set` / `ipset -N` commands
at the shell are not sufficient for production use because they are lost
on reload.

---

### Notes

* On Linux, root privileges (or `CAP_NET_ADMIN`) are required for both
  backends. When Unbound drops privileges via the `username:` setting, it
  automatically preserves `CAP_NET_ADMIN` (`PR_SET_KEEPCAPS`) if the
  module is configured.
* On BSD, Unbound requires read/write access to `/dev/pf`. If dropping
  privileges, ensure the Unbound user is in a group with access to the
  device.
* `name-v4` / `name-v6` are bare set names. For the nftset backend, the
  table that contains them is given separately in `table:`.
* Only one backend is active per Unbound process — the configuration
  section used (`ipset:` vs `nftset:`) selects which is active.
* On BSD only the `ipset:` section is supported, this populates PF tables.
