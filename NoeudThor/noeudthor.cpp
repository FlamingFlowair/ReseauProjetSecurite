#include "noeudthor.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>

NoeudThor::NoeudThor(boost::asio::io_service &io_service, int portecoute)
	: portecoute(portecoute),
	  m_acceptor(io_service, tcp::endpoint(tcp::v4(), portecoute)),
	  io_service(io_service)
{
	cout << "Construction d'un noeudThor" << std::endl;
	startConnect();
	startAccept();
	giveEarPort();
	askNeighborList();
	askNombreNoeuds();
}


void NoeudThor::startConnect()
{
	noeudServeurCentral = new Client<NoeudThor>(this, io_service);
	tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 8080);
	cout << "Connection au serveur central" << std::endl;
	noeudServeurCentral->getSocket().connect(endpoint);
	cout << "Connection effectuée on lance la lecture" << std::endl;
	noeudServeurCentral->startRead();
}

void NoeudThor::giveEarPort(){
	cout << "Le NoeudThor demande la liste d'ip:portd'ecoute des autres noeudsThors / frontales" << std::endl;
	Trame t(-1, std::to_string(portecoute));
	cout << "/*********" << std::endl <<
			"TTL : " << t.getTTL() << std::endl <<
			"Commande : " << t.getCommande() << std::endl <<
			"*********/" << std::endl;
	boost::asio::streambuf buf;
	ostream os(&buf);
	boost::archive::binary_oarchive archiveBinaire(os);
	archiveBinaire << t;

	noeudServeurCentral->send(t);
}

void NoeudThor::askNeighborList(){
	cout << "Le NoeudThor demande la liste d'ip:portd'ecoute des autres noeudsThors / frontales" << std::endl;
	Trame t(-2, "");
	cout << "/*********" << std::endl <<
			"TTL : " << t.getTTL() << std::endl <<
			"Commande : " << t.getCommande() << std::endl <<
			"*********/" << std::endl;
	boost::asio::streambuf buf;
	ostream os(&buf);
	boost::archive::binary_oarchive archiveBinaire(os);
	archiveBinaire << t;

	noeudServeurCentral->send(t);
	//boost::asio::write(to->getSocket(), buf);
	//cout << "Write effectué vers le client" << std::endl;
	//boost::asio::write(to->getSocket(), boost::asio::buffer(string("Voici une très longue string pour montrer que le serveur est capable de parler au client")));
}


void NoeudThor::askNombreNoeuds(){
	cout << "Le NoeudThor demande le nombre de noeuds du réseau" << std::endl;
	Trame t(-3, "");
	cout << "/*********" << std::endl <<
			"TTL : " << t.getTTL() << std::endl <<
			"Commande : " << t.getCommande() << std::endl <<
			"*********/" << std::endl;
	boost::asio::streambuf buf;
	ostream os(&buf);
	boost::archive::binary_oarchive archiveBinaire(os);
	archiveBinaire << t;

	noeudServeurCentral->send(t);
}

void NoeudThor::clientLeave(Client<NoeudThor> *leaving)
{
	auto i = std::begin(this->toutlemonde);

	while (i != std::end(this->toutlemonde)) {
		if (*i == leaving)
			i = this->toutlemonde.erase(i);
		else
			++i;
	}
}

void NoeudThor::startAccept()
{
	cout << "Le NoeudThor lance l'acceptation de nouveaux Noeuds" << std::endl;
	Client<NoeudThor>* newNoeud = new Client<NoeudThor>(this, io_service);

	m_acceptor.async_accept(newNoeud->getSocket(),
		boost::bind(&NoeudThor::handle_accept, this, newNoeud,
		boost::asio::placeholders::error));
}

void NoeudThor::traitementDeLaTrame(Trame &t, Client<NoeudThor> *noeudSource)
{
	cout << "Debut " << __FUNCTION__ << std::endl;
	if (noeudSource == this->noeudServeurCentral){
		cout << "If " << __FUNCTION__ << std::endl;
		switch (t.getTTL()) {
			case -1:
			{
				cout << "Really ?" << std::endl;
				break;
			}
			case -2:
			{
				cout << "On a recu la liste d'ip:portd'écoute des autres clients" << std::endl;
				list<pair <string, int> > ipPortEveryBody;

				std::istringstream iStringStream(t.getCommande());
				boost::archive::text_iarchive iTextArchive(iStringStream);
				iTextArchive >> ipPortEveryBody;

				for(pair <string, int> trucl : ipPortEveryBody){
					cout << "IP : " << trucl.first << std::endl <<
							"Port : " << trucl.second << std::endl;
				}
				cout << "Enregistrement des différents noeuds effectué" << std::endl;
				break;
			}
			case -3:
			{
				cout << "On a recu le nombre de Noeuds du réseau" << std::endl;
				int nombre;
				sscanf(t.getCommande().c_str(), "%d", &nombre);
				cout << "Il y a " << nombre << "noeuds sur le réseau." << std::endl;
				break;
			}
			default:
			{
				cout << "On a recu un truc mais un code incohérent" << std::endl;
				break;
			}
		}
	}
	else{
		cout << "Else " << __FUNCTION__ << std::endl;
	}
	cout << "Fin " << __FUNCTION__ << std::endl;
}

void NoeudThor::handle_accept(Client<NoeudThor> *noeud, const boost::system::error_code &error)
{
	if (!error)
	{
		std::cout << "Nouveau Client se connecte !" << std::endl;
		cout << "On ajoute le Client à la liste des Clients connus" << std::endl;
		toutlemonde.push_back(noeud);
		cout << "On lance la lecture async sur le Client" << std::endl;
		noeud->startRead();
		cout << "On relance l'acceptation de nouveau Clients" << std::endl;
		startAccept(); // (5)
	}
}
