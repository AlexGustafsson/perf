FROM gcc:9

WORKDIR /src

RUN apt-get update && apt-get install --no-install-recommends -y libcap-dev && rm -rf /var/cache/apt/archives /var/lib/apt/lists/*

COPY . .

RUN make

WORKDIR /src/build/examples