#!/bin/bash

cpplint --filter=-legal/copyright,-build/include,-build/c++11,-runtime/references \
        --repository=. \
        --root=include \
        --extensions=hpp,cpp \
        --recursive \
        ./bench ./src ./tests ./include

echo "Done!"
