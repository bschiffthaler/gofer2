#!/bin/bash

curl ftp://ftp.ebi.ac.uk/pub/databases/Pfam/releases/Pfam31.0/database_files/pfamA.txt.gz | gzip -d -c - | cut -f 1,4,9 > pfam.txt
