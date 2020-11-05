# -*- mode: ruby -*-
# vi: set ft=ruby :

$script = <<-SCRIPT
sudo add-apt-repository ppa:beineri/opt-qt-5.12.0-xenial
sudo apt-get update
sudo apt-get install build-essential qt512base qt512declarative
source /opt/qt512/bin/qt*-env.sh
SCRIPT

Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/xenial64"
  config.vm.provision "shell", inline: $script
end
