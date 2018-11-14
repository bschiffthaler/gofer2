from ftplib import FTP
import tempfile
import gzip

ftp = FTP("ftp.ebi.ac.uk")
ftp.login()

dead_file = "RETR pub/databases/Pfam/releases/Pfam32.0/Pfam-A.dead.gz"
full_file = "RETR pub/databases/Pfam/releases/Pfam32.0/database_files/pfamA.txt.gz"

# The current annotation is stored in a TSV file. Interesting fields are
# 0: The PFAM ID
# 3: The short name
# 8: The full description
tmp_full = tempfile.TemporaryFile()
ftp.retrbinary(full_file, tmp_full.write)

tmp_full.seek(0)

with gzip.open(tmp_full) as gz:
  for line in gz:
    fields = line.decode().strip().split("\t")
    print(fields[0], fields[3], fields[7], sep = "\t")

# Also add dead terms for completeness, since many annotations tend not to be
# updated frequently enough to not contain any outdated terms
tmp_dead = tempfile.TemporaryFile()
ftp.retrbinary(dead_file, tmp_dead.write)

tmp_dead.seek(0)

with gzip.open(tmp_dead) as gz:
  AC = ""
  CC = ""
  for line in gz:
    l = line.decode().strip()
    if l.startswith("#=GF AC"):
      AC = " ".join(l.split()[2:])
    elif l.startswith("#=GF CC"):
      CC = CC + " " + " ".join(l.split()[2:])
    elif l.startswith("//"):
      print(AC, "(Dead): " + AC, "(Dead): " + CC, sep = "\t")
      AC = ""
      CC = ""

tmp_dead.close()

