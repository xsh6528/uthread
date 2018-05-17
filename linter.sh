#!/bin/bash

cpplint --filter=-legal/copyright,-build/include,-build/c++11,-runtime/references \
        --repository=. \
        --root=include \
        --extensions=hpp,cpp,h,c \
        --recursive \
        ./bench ./src ./tests ./include

echo "Done!"
