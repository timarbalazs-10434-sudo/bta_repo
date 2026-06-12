import { Routes } from '@angular/router';

export const routes: Routes = [
    {
        path : "",
        pathMatch : "full",
        loadComponent: () =>{
            return import('./home/home').then(
                (m) => m.Home
            )
        }
    },
    {
        path : "download",
        loadComponent : () =>{
            return import('./download/download').then(
                (m) => m.Download
            )
        }
    },
    {
        path : "inventory",
        loadComponent : () =>{
            return import('./inventory/inventory').then(
                (m) => m.Inventory
            )
        }
    },
    {
        path: "iteminventory",
        loadComponent : () =>{
            return import('./components/item-inventory/item-inventory').then(
                (m) => m.ItemInventory
            )
        }
    },
    {
        path: "**", 
        redirectTo: ""
    }
];
