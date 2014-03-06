#include "Camera.h"

void Camera::scrollCamera(GameObject * obj, sf::View * view){
	if (obj->getPosition().x + 15 > view->getCenter().x){ // && obj->getPosition().x < MAX - 80
		view->setCenter(view->getCenter().x + (obj->getPosition().x - view->getCenter().x + 15), view->getCenter().y);
	}
	if (view->getCenter().x > obj->getPosition().x && obj->getPosition().x > 80){
		view->setCenter(view->getCenter().x + (obj->getPosition().x - view->getCenter().x + 15), view->getCenter().y);
	}
}