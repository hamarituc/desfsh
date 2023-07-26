result = buf.th(crypto.cmac("AES-128-CBC", "01020202020202023f333355987654020202020202023f333355987654", "00112233445566778899aabbccddeeff"))
print(result)
assert(result == "39d7b23f8b3cc670d249df313968bbce")

result = buf.th(key.div({ t = "AES", k = "00112233445566778899aabbccddeeff", v = 0 }, "02020202020202", "0x3f3333", "0x55", "987654")["k"])
print(result)
assert(result == "39d7b23f8b3cc670d249df313968bbce")
