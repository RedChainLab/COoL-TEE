-include secrets.mk

SRC_GROUP?=src-group
LOCATION?=location

AUTHORIZED_IP?=IP
PUB_KEY?=pathtokey.pub
USER?=user

PREFIX?=cooltee-

VNET_SUFFIX?=vnet
SUBNET_SUFFIX?=subnet
NSG_SUFFIX?=nsg

VNET_NAME=${PREFIX}${VNET_SUFFIX}

MARKET_VM_TYPE?=Standard_B4als_v2
CONSUMER_VM_TYPE?=Standard_B16als_v2
PROVIDER_VM_TYPE?=Standard_DC4s_v2
PRIORITY?=Spot

src-group:
	az group create --name ${SRC_GROUP} --location ${LOCATION}

nsg:
	az network nsg create --resource-group ${SRC_GROUP} --name ${PREFIX}${NSG_SUFFIX}
	az network nsg rule create --resource-group ${SRC_GROUP} --nsg-name ${PREFIX}${NSG_SUFFIX} --name allow-ip --destination-port-ranges '22' --priority 1000 --access Allow

vnet:
	az network vnet create --name ${VNET_NAME} --resource-group ${SRC_GROUP}  --address-prefixes 10.0.0.0/16
	az network vnet subnet create --name ${PREFIX}market-${SUBNET_SUFFIX} --address-prefixes 10.0.0.0/24 --nsg ${PREFIX}${NSG_SUFFIX} --vnet-name ${VNET_NAME} --resource-group ${SRC_GROUP}
	az network vnet subnet create --name ${PREFIX}prov-hon-${SUBNET_SUFFIX} --address-prefixes 10.0.1.0/24 --nsg ${PREFIX}${NSG_SUFFIX} --vnet-name ${VNET_NAME} --resource-group ${SRC_GROUP}
	az network vnet subnet create --name ${PREFIX}prov-mal-${SUBNET_SUFFIX} --address-prefixes 10.0.2.0/24 --nsg ${PREFIX}${NSG_SUFFIX} --vnet-name ${VNET_NAME} --resource-group ${SRC_GROUP}
	az network vnet subnet create --name ${PREFIX}req-hon-${SUBNET_SUFFIX} --address-prefixes 10.0.3.0/24 --nsg ${PREFIX}${NSG_SUFFIX} --vnet-name ${VNET_NAME} --resource-group ${SRC_GROUP}
	az network vnet subnet create --name ${PREFIX}req-mal-${SUBNET_SUFFIX} --address-prefixes 10.0.4.0/24 --nsg ${PREFIX}${NSG_SUFFIX} --vnet-name ${VNET_NAME} --resource-group ${SRC_GROUP} 

new-all: $(patsubst config/${PREFIX}%.json,new-%,$(wildcard config/*.json))

new-market:
	$(eval VM_NAME=${PREFIX}$(subst new-,,$@))
	az vm create \
		--resource-group ${SRC_GROUP} \
		--name ${VM_NAME} \
		--image Ubuntu2204 \
		--size ${MARKET_VM_TYPE} \
		--authentication-type ssh \
		--os-disk-delete-option Delete \
		--ssh-key-value ${PUB_KEY} \
		--admin-username ${USER} \
		--nsg ${PREFIX}${NSG_SUFFIX} \
		--vnet-name ${VNET_NAME} \
		--subnet ${PREFIX}market-${SUBNET_SUFFIX} \
		--public-ip-sku Standard \
		> config/${VM_NAME}.json	
	az vm auto-shutdown \
  		--resource-group ${SRC_GROUP} \
  		--name ${VM_NAME} \
 		--time 1800
	echo "" > ~/.ssh/known_hosts
	sleep 1
	sudo sed -i "s/.*${VM_NAME}/$(shell jq -r '.publicIpAddress' config/${VM_NAME}.json)    ${VM_NAME}/g" /etc/hosts
	sleep 1
	scp -o StrictHostKeyChecking=no -r ../../deployed ${USER}@${VM_NAME}:/home/${USER}/${PREFIX}deployed

new-prov-%:
	$(eval VM_NAME=${PREFIX}$(subst new-,,$@))
	$(eval ROLE=prov-$(word 3, $(subst -, ,$@)))
	az vm create \
		--resource-group ${SRC_GROUP} \
		--name ${VM_NAME} \
		--image Canonical:0001-com-ubuntu-server-focal:20_04-lts-gen2:latest \
		--size ${PROVIDER_VM_TYPE} \
		--authentication-type ssh \
		--os-disk-delete-option Delete \
		--ssh-key-value ${PUB_KEY} \
		--admin-username ${USER} \
		--nsg ${PREFIX}${NSG_SUFFIX} \
		--vnet-name ${VNET_NAME} \
		--subnet ${PREFIX}${ROLE}-${SUBNET_SUFFIX} \
		--public-ip-sku Standard \
		> config/${VM_NAME}.json	
	az vm auto-shutdown \
  		--resource-group ${SRC_GROUP} \
  		--name ${VM_NAME} \
 		--time 1800
	echo "" > ~/.ssh/known_hosts
	sleep 1
	sudo sed -i "s/.*${VM_NAME}/$(shell jq -r '.publicIpAddress' config/${VM_NAME}.json)    ${VM_NAME}/g" /etc/hosts
	sleep 1
	scp -o StrictHostKeyChecking=no -r ../../deployed ${USER}@${VM_NAME}:/home/${USER}/${PREFIX}deployed
	ssh ${USER}@${VM_NAME} "cd ~/${PREFIX}deployed/out && sh setup.sh"

new-req-%:
	$(eval VM_NAME=${PREFIX}$(subst new-,,$@))
	$(eval ROLE=req-$(word 3, $(subst -, ,$@)))
	az vm create \
		--resource-group ${SRC_GROUP} \
		--name ${VM_NAME} \
		--image Ubuntu2204 \
		--size ${CONSUMER_VM_TYPE} \
		--authentication-type ssh \
		--os-disk-delete-option Delete \
		--ssh-key-value ${PUB_KEY} \
		--admin-username ${USER} \
		--nsg ${PREFIX}${NSG_SUFFIX} \
		--vnet-name ${VNET_NAME} \
		--subnet ${PREFIX}${ROLE}-${SUBNET_SUFFIX} \
		--public-ip-sku Standard \
		> config/${VM_NAME}.json	 
	az vm auto-shutdown \
  		--resource-group ${SRC_GROUP} \
  		--name ${VM_NAME} \
 		--time 1800
	echo "" > ~/.ssh/known_hosts
	sleep 1
	sudo sed -i "s/.*${VM_NAME}/$(shell jq -r '.publicIpAddress' config/${VM_NAME}.json)    ${VM_NAME}/g" /etc/hosts
	sleep 1
	scp -o StrictHostKeyChecking=no -r ../../deployed ${USER}@${VM_NAME}:/home/${USER}/${PREFIX}deployed

del-src-group:
	az group delete --name ${SRC_GROUP} --yes

del-vnet:
	az network vnet delete --name ${VNET_NAME} --resource-group ${SRC_GROUP}

del-nsg:
	az network nsg delete --name ${PREFIX}${NSG_SUFFIX} --resource-group ${SRC_GROUP}

start-all: $(patsubst config/${PREFIX}%.json,start-%,$(wildcard config/*.json))

stop-all: $(patsubst config/${PREFIX}%.json,stop-%,$(wildcard config/*.json))

restart-all: $(patsubst config/${PREFIX}%.json,restart-%,$(wildcard config/*.json))

del-all: $(patsubst config/${PREFIX}%.json,del-%,$(wildcard config/*.json))

del-%:
	$(eval VM_NAME=${PREFIX}$(subst del-,,$@))
	az vm delete --name ${VM_NAME} --resource-group ${SRC_GROUP} --yes
	az network nic ip-config update --name ipconfig${VM_NAME} --nic-name ${VM_NAME}VMNic --resource-group ${SRC_GROUP} --remove publicIpAddress
	az network public-ip delete --name ${VM_NAME}PublicIP --resource-group ${SRC_GROUP}
	az network nic delete --name ${VM_NAME}VMNic --resource-group ${SRC_GROUP}

restart-%:
	$(eval VM_NAME=${PREFIX}$(subst restart-,,$@))
	az vm restart --name ${VM_NAME} \
		--resource-group ${SRC_GROUP} \
		--force

start-%:
	$(eval VM_NAME=${PREFIX}$(subst start-,,$@))
	az vm start --name ${VM_NAME} \
		--resource-group ${SRC_GROUP}

stop-%:
	$(eval VM_NAME=${PREFIX}$(subst stop-,,$@))
	az vm stop --name ${VM_NAME} \
		--resource-group ${SRC_GROUP}

deallocate-%:
	$(eval VM_NAME=${PREFIX}$(subst start-,,$@))
	az vm deallocate --name ${VM_NAME} \
		--resource-group ${SRC_GROUP}

.PHONY: src-group vnet nsg start-all stop-all restart-all del-vnet del-nsg del-all new-all