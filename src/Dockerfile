From gcc:9

RUN mkdir -p /code
WORKDIR /code
add . /code/

ENTRYPOINT ["taskset --cpu-list 0-3 ./run.sh"]
