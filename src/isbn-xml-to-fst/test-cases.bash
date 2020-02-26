#! /bin/bash

# Date: 2017-10-14 , 2018-09-23
#
# 9780312128470
#     Agency="English Language"
#     Expect=978-0-312-12847-0
#
# 9788132220794
#     Agency=India
#     Expect=978-81-322-2079-4
#
# 9780131103627
#     Agency="English Language"
#     Expect=978-0-13-110362-7
#
# 9781449373320
#     Agency=???
#     Expect=978-1-4493-7332-0
#
#

cd ~/vproject/isbn/ws/src/isbn-xml-to-fst || exit 2

rm -f core
rm -rf tmp
mkdir tmp

for isbn in 9780312128470 9788132220794 9780131103627 9781449373320
do
    ./isbn-xml-to-fst isbn-range.xml "${isbn}" \
    > tmp/${isbn}.out 2>tmp/${isbn}.err
done

exit 0
