#!/bin/sh

DISABLED_TESTS=(\
    preset_pinning \
    utf8_open \
    sf3_sfont_loading \
    sample_cache \
    sfont_loading \
    preset_sample_loading \
    sfont_unloading \
    sample_rate_change \
    bug_635 \
    )

rm -f test-names.txt
mkdir -p app/src/main/cpp/tests/

for f in `grep -lR "int main(void)" ../test/ | sort` ; do
    export TESTMAINNAME=`echo $f | sed -e "s/\.\.\/test\/test_\(.*\).c$/\1/"`
    echo $TESTMAINNAME >> test-names.txt
    export OUTPUTFILE=app/src/main/cpp/tests/test_${TESTMAINNAME}.c
    sed -e "s/int main(void)/int "$TESTMAINNAME"_main(void)/" $f > $OUTPUTFILE ;
done

RUN_ALL_TESTS=app/src/main/cpp/run_all_tests.c

rm -f $RUN_ALL_TESTS

while IFS= read -r line; do
    echo "int "$line"_main();" >> $RUN_ALL_TESTS ;
done < test-names.txt

echo "int run_all_fluidsynth_tests() {" >> $RUN_ALL_TESTS
echo "    int ret = 0; " >> $RUN_ALL_TESTS

while IFS= read -r line; do
    if [[ " ${DISABLED_TESTS[@]} " =~ " ${line} " ]]; then
        echo "    //ret += "$line"_main();" >> $RUN_ALL_TESTS ;
    else
        echo "    ret += "$line"_main();" >> $RUN_ALL_TESTS ;
    fi
done < test-names.txt

echo "    return ret;" >> $RUN_ALL_TESTS
echo "}" >> $RUN_ALL_TESTS
