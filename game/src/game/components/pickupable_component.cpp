#include "pch.h"

#include "pickupable_component.h"

void PickupableComponent::init()
{

}

void PickupableComponent::update()
{
}

void PickupableComponent::on_pickup()
{
	is_held = true;
}

void PickupableComponent::on_drop()
{
	is_held = false;
}