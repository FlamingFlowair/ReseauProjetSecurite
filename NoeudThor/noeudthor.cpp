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
}


void NoeudThor::startConnect()
{
	noeudServeurCentral = new Client<NoeudThor>(this, io_service);
	tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 8080);
	cout << "connection au serveur central" << std::endl;
	noeudServeurCentral->getSocket().connect(endpoint);
	noeudServeurCentral->startRead();
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
	cout << "Trame Recue" << std::endl;
	if (t.getTTL() == -1 && noeudSource == this->noeudServeurCentral){
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
	}
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
