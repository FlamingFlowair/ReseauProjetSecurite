#include "serveurcentral.h"
#include <boost/bind.hpp>
#include <iostream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>

ServeurCentral::ServeurCentral(boost::asio::io_service &io_service, int portecoute)
	: portecoute(portecoute),
	  m_acceptor(io_service, tcp::endpoint(tcp::v4(), portecoute)),
	  io_service(io_service)
{
	cout << "Construction du serveur central" << std::endl;
	startAccept();
}

void ServeurCentral::startAccept()
{
	cout << "Le serveur central lance l'acceptation de nouveaux Noeuds" << std::endl;
	Client<ServeurCentral>* newNoeud = new Client<ServeurCentral>(this, io_service);
	m_acceptor.async_accept(newNoeud->getSocket(),
		boost::bind(&ServeurCentral::handle_accept,
					this,
					newNoeud,
					boost::asio::placeholders::error));
}

void ServeurCentral::handle_accept(Client<ServeurCentral> *noeud, const boost::system::error_code &error)
{
	if (!error)
	{
		std::cout << "Nouveau Client se connecte !" << std::endl;

		cout << "On ajoute le Client à la liste des Clients connus" << std::endl;
		toutlemonde.push_back(noeud);
		cout << "On lance la lecture async sur le Client" << std::endl;
		noeud->startRead();
		startAccept();
	}
}

void ServeurCentral::sendIpPortAllNodes(Client<ServeurCentral> *to){
	cout << "Le serveur central renvoie au noeud la liste d'ip:portd'ecoute des clients connetés" << std::endl;
	std::ostringstream oStringStream;
	boost::archive::text_oarchive oTextArchive(oStringStream);

	list<pair <string, int> > ipPortEveryBody;
	for(auto cl : this->toutlemonde){
		Client<ServeurCentral> *cli = cl;
		ipPortEveryBody.insert(ipPortEveryBody.end(), pair<string, int>(cli->getIpStr(), cli->getPort()));
		printf("CLI\n");
	}
	oTextArchive << ipPortEveryBody;    // sérialisation de t

	Trame t(-2, oStringStream.str());
	cout << "/*********" << std::endl <<
			"TTL : " << t.getTTL() << std::endl <<
			"Commande : " << t.getCommande() << std::endl <<
			"*********/" << std::endl;
	boost::asio::streambuf buf;
	ostream os(&buf);
	boost::archive::binary_oarchive archiveBinaire(os);
	archiveBinaire << t;

	to->send(t);
	//boost::asio::write(to->getSocket(), buf);
	//cout << "Write effectué vers le client" << std::endl;
	//boost::asio::write(to->getSocket(), boost::asio::buffer(string("Voici une très longue string pour montrer que le serveur est capable de parler au client")));
}

void ServeurCentral::sendNbNoeuds(Client<ServeurCentral> *to){
	cout << "Le serveur central envoie le nombre de noeuds du réseau" << std::endl;
	list<pair <string, int> > ipPortEveryBody;
	for(auto cl : this->toutlemonde){
		Client<ServeurCentral> *cli = cl;
		if (cli->getPort() != 0)
			ipPortEveryBody.insert(ipPortEveryBody.end(), pair<string, int>(cli->getIpStr(), cli->getPort()));
	}

	Trame t(-3, std::to_string(ipPortEveryBody.size()));
	cout << "/*********" << std::endl <<
			"TTL : " << t.getTTL() << std::endl <<
			"Commande : " << t.getCommande() << std::endl <<
			"*********/" << std::endl;
	boost::asio::streambuf buf;
	ostream os(&buf);
	boost::archive::binary_oarchive archiveBinaire(os);
	archiveBinaire << t;

	to->send(t);
}

void ServeurCentral::traitementDeLaTrame(Trame &t, Client<ServeurCentral> *noeudSource)
{
	cout << "Traitement de la trame" << std::endl;
	const int TTL_RENSEIGNE_NO_PORT=-1;
	const int TTL_ASK_LIST_IP_PORT=-2;
	const int TTL_ASK_NB_NOEUDS=-3;
	/*Si le noeud est complété*/
	if (noeudSource->getPort() == 0){
		if (t.getTTL() != TTL_RENSEIGNE_NO_PORT){
			cout << "Erreur un client essaye de communiquer sans avoir de port d'écoute ouvert/renseigné" << std::endl;
			return;
		}
		else {
			noeudSource->setPort(stoi(t.getCommande(), NULL, 10));
			cout << "Le noeud à renseigné son port" << std::endl;
		}
	}
	else {
		cout << "ELSEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
		switch (t.getTTL()) {
			case TTL_ASK_LIST_IP_PORT:
				cout << "Le noeud demande la liste d'ip:portd'écoute" << std::endl;
				sendIpPortAllNodes(noeudSource);
				break;
			case TTL_ASK_NB_NOEUDS:
				sendNbNoeuds(noeudSource);
				break;
			default:
				cout << "Le noeud demande le broadcast d'un message" << std::endl;
				sendTrameToRecipient(t);
				break;
		}
	}
}

void ServeurCentral::sendTrameToRecipient(Trame &t)
{
	t.setTTL(0);
	for (auto n : toutlemonde){
		Client<ServeurCentral>* p = n;
		p->send(t);
	}
}

void ServeurCentral::clientLeave(Client<ServeurCentral> *leaving)
{
	auto i = std::begin(this->toutlemonde);

	while (i != std::end(this->toutlemonde)) {
		if (*i == leaving)
			i = this->toutlemonde.erase(i);
		else
			++i;
	}
}
