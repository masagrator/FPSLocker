from urllib.request import Request, urlopen

titleids = []

site = "https://raw.githubusercontent.com/masagrator/FPSLocker-Warehouse/refs/heads/v4/README.md"
request_site = Request(site, headers={"User-Agent": "Mozilla/5.0"})
text = urlopen(request_site).read().decode("UTF-8").split("\n")

for line in text:
    if line.find("| `0100") == -1:
        continue
    if line.find("(◯,") != -1:
        continue
    pos = line.find("| `0100") + 3
    titleid = line[pos:pos+16].upper()
    titleids.append(int.from_bytes(bytes.fromhex(titleid)))

new_file = open("source/titleids_with_patches.bin", "wb")
for i in range(len(titleids)):
    new_file.write(titleids[i].to_bytes(8, "little"))

new_file.close()
