#ifndef NOEUDTHOR_H
#define NOEUDTHOR_H

#include <boost/serialization/list.hpp>
#include "client.h"
#include <string>

class NoeudThor
{
	private:
		int portecoute;
		std::list<Client<NoeudThor>*> toutlemonde;
		tcp::acceptor m_acceptor;
		boost::asio::io_service &io_service;
		Client<NoeudThor>* noeudServeurCentral;

		void handle_accept(Client<NoeudThor> *noeud, const boost::system::error_code &error);
		void handle_read(Client<NoeudThor> *noeud, const boost::system::error_code &error);
		void startConnect();
	public:
		NoeudThor(boost::asio::io_service &io_service, int portecoute);
		void startAccept();
		void traitementDeLaTrame(Trame &t, Client<NoeudThor> *noeudSource);
};
#endif //NOEUDTHOR_H
