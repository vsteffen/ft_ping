FROM amd64/ubuntu:bionic

CMD ["/bin/bash"]

RUN rm /etc/dpkg/dpkg.cfg.d/excludes

RUN apt update \
	&& apt install -y git vim man gcc clang make iputils-ping \
	&& mkdir /root/ft_ping

WORKDIR /root/ft_ping
