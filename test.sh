
./10cc test.c > tmp.s
if [ $? != 0 ]; then
  echo "Failed to compile test.c by 10cc"
  exit
fi
cc -no-pie -o tmp.o tmp.s test_mock.s
if [ $? != 0 ]; then
  echo "Failed to compile tmp.s by cc"
  exit
fi
./tmp.o
if [ $? != 0 ]; then
  echo "Failed to execute tmp.o"
  exit
fi
