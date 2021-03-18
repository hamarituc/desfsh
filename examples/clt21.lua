debugset(0)
cmd.select(0)

-- DES-Karten bei Bedarf in den AES-Modus schalten.
result, errmsg = cmd.auth(0, DES())
if result == 0 then
  cmd.ck(0, AES(), DES())
end

debugset(255)
result, errmsg = cmd.auth(0, AES())
assert(result, errmsg)

-- Hardware-Reset
cmd.format()

cmd.capp("AES", 1, 0x0f, 4)
cmd.select(1)
cmd.auth(0, AES())
cmd.ck(1, AES("11111111111111111111111111111111"), AES())
cmd.ck(2, AES("22222222222222222222222222222222"), AES())
cmd.ck(3, AES("33333333333333333333333333333333"), AES())

cmd.csdf(0, "CRYPT", { rd = 1, wr = 2, rw = 3, ca = 0 }, 32)
cmd.cbdf(1, "CRYPT", { rd = 1, wr = 2, rw = 3, ca = 0 }, 32)
cmd.clrf(2, "CRYPT", { rd = 1, wr = 2, rw = 3, ca = 0 }, 8, 4)
cmd.ccrf(3, "CRYPT", { rd = 1, wr = 2, rw = 3, ca = 0 }, 8, 4)
cmd.cvf( 4, "CRYPT", { rd = 1, wr = 2, rw = 3, ca = 0 }, 0, 100, 0, true)

cmd.auth(3, AES("33333333333333333333333333333333"))
cmd.write(0, 0, buf.fa("Chemnitzer"), "CRYPT")
cmd.write(1, 0, buf.fa("Linux-Tage"), "CRYPT")
cmd.commit()
cmd.wrec(2, 0, buf.fa("CLT"), "CRYPT")
cmd.commit()
cmd.wrec(2, 1, buf.fa("2021"), "CRYPT")
cmd.commit()
cmd.wrec(3, 0, buf.fa("CLT"), "CRYPT")
cmd.commit()
cmd.wrec(3, 1, buf.fa("2021"), "CRYPT")
cmd.commit()
cmd.credit(4, 20)
cmd.commit()
cmd.credit(4, 0)
cmd.commit()
